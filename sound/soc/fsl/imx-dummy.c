/* i.MX Dummy audio support
 *
 * Copyright 2018 NXP
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/gpio/consumer.h>
#include <linux/of_device.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <sound/soc-dapm.h>

#include "fsl_sai.h"

struct imx_dummy_data {
	struct snd_soc_card card;
	unsigned long freq;
};

static struct snd_soc_dapm_widget imx_dummy_dapm_widgets[] = {
	SND_SOC_DAPM_LINE("Line Out", NULL),
};

static unsigned long imx_dummy_compute_freq(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct imx_dummy_data *priv = snd_soc_card_get_drvdata(rtd->card);

	return priv->freq;
}

static int imx_dummy_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = asoc_rtd_to_codec(rtd, 0);
	struct snd_soc_card *card = rtd->card;
	struct device *dev = card->dev;
	unsigned int channels = params_channels(params);
	unsigned int fmt = SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS;
	unsigned long freq = imx_dummy_compute_freq(substream, params);
	int ret;

	fmt |= SND_SOC_DAIFMT_I2S;

	ret = snd_soc_dai_set_sysclk(cpu_dai, FSL_SAI_CLK_MAST1, freq,
					SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		dev_err(dev, "failed to set cpu dai mclk1 rate(%lu): %d\n",
			freq, ret);
		return ret;
	}

	ret = snd_soc_dai_set_fmt(cpu_dai, fmt);
	if (ret) {
		dev_err(dev, "failed to set cpu dai fmt: %d\n", ret);
		return ret;
	}

	ret = snd_soc_dai_set_tdm_slot(cpu_dai,
			       BIT(channels) - 1, BIT(channels) - 1,
			       2, params_physical_width(params));
	if (ret) {
		dev_err(dev, "failed to set cpu dai tdm slot: %d\n", ret);
		return ret;
	}

	return ret;
}

static const u32 support_rates[] = {
	8000, 11025, 12000, 16000, 22050,
	24000, 32000, 44100, 48000, 64000,
	88200, 96000, 176400, 192000, 352800,
	384000, 705600, 768000, 1411200, 2822400,
};

static const u32 support_channels[] = {
	1, 2, 4, 6, 8, 10, 12, 14, 16,
};

static int imx_dummy_startup(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	int ret = 0;
	static struct snd_pcm_hw_constraint_list constraint_rates;
	static struct snd_pcm_hw_constraint_list constraint_channels;

	constraint_rates.list = support_rates;
	constraint_rates.count = ARRAY_SIZE(support_rates);

	constraint_channels.list = support_channels;
	constraint_channels.count = ARRAY_SIZE(support_channels);

	ret = snd_pcm_hw_constraint_list(runtime, 0, SNDRV_PCM_HW_PARAM_CHANNELS,
						&constraint_channels);
	if (ret)
		return ret;

	ret = snd_pcm_hw_constraint_list(runtime, 0, SNDRV_PCM_HW_PARAM_RATE,
						&constraint_rates);
	if (ret)
		return ret;

	return ret;
}

static struct snd_soc_ops imx_aif_ops = {
	.startup   = imx_dummy_startup,
	.hw_params = imx_dummy_hw_params,
};

SND_SOC_DAILINK_DEFS(hifi,
	DAILINK_COMP_ARRAY(COMP_EMPTY()),
	DAILINK_COMP_ARRAY(COMP_EMPTY()),
	DAILINK_COMP_ARRAY(COMP_EMPTY()));

static struct snd_soc_dai_link imx_dummy_dai = {
	.name = "dummy",
	.stream_name = "Audio",
	.ops = &imx_aif_ops,
	SND_SOC_DAILINK_REG(hifi),
};

static int imx_dummy_probe(struct platform_device *pdev)
{
	const char *codec_dai_name;
	struct imx_dummy_data *priv;
	struct device_node *cpu_np, *codec_np = NULL;
	struct platform_device *cpu_pdev;
	struct clk *mclk;
	int ret;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	cpu_np = of_parse_phandle(pdev->dev.of_node, "audio-cpu", 0);
	if (!cpu_np) {
		dev_err(&pdev->dev, "audio dai phandle missing or invalid\n");
		ret = -EINVAL;
		goto fail;
	}

	codec_np = of_parse_phandle(pdev->dev.of_node, "audio-codec", 0);
	if (!codec_np) {
		dev_err(&pdev->dev, "audio codec phandle missing or invalid\n");
		ret = -EINVAL;
		goto fail;
	}

	cpu_pdev = of_find_device_by_node(cpu_np);
	if (!cpu_pdev) {
		dev_err(&pdev->dev, "failed to find SAI platform device\n");
		ret = -EINVAL;
		goto fail;
	}

	if (of_device_is_compatible(codec_np, "linux,snd-soc-dummy")) {
		codec_dai_name = "snd-soc-dummy-dai";
	} else if (of_device_is_compatible(codec_np, "linux,dummy-codec")) {
		codec_dai_name = "dummy-codec-dai";
	} else {
		dev_err(&pdev->dev, "unknown Device Tree compatible\n");
		ret = -EINVAL;
		goto fail;
	}

	imx_dummy_dai.codecs->dai_name = codec_dai_name;
	imx_dummy_dai.codecs->of_node = codec_np;
	imx_dummy_dai.cpus->dai_name = dev_name(&cpu_pdev->dev);
	imx_dummy_dai.cpus->of_node = cpu_np;
	imx_dummy_dai.platforms->of_node = cpu_np;

	priv->card.dai_link = &imx_dummy_dai;
	priv->card.num_links = 1;
	priv->card.dev = &pdev->dev;
	priv->card.owner = THIS_MODULE;
	priv->card.dapm_widgets = imx_dummy_dapm_widgets;
	priv->card.num_dapm_widgets = ARRAY_SIZE(imx_dummy_dapm_widgets);

	mclk = devm_clk_get(&cpu_pdev->dev, "mclk1");
	if (IS_ERR_OR_NULL(mclk)) {
		dev_err(&pdev->dev, "failed to get DAI mclk1 (%ld)\n", PTR_ERR(mclk));
		ret = -EINVAL;
		goto fail;
	}

	priv->freq = clk_get_rate(mclk);

	ret = snd_soc_of_parse_card_name(&priv->card, "model");
	if (ret)
		goto fail;

	snd_soc_card_set_drvdata(&priv->card, priv);

	ret = devm_snd_soc_register_card(&pdev->dev, &priv->card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card failed (%d)\n", ret);
		goto fail;
	}

	ret = 0;
fail:
	if (cpu_np)
		of_node_put(cpu_np);
	if (codec_np)
		of_node_put(codec_np);
	if (cpu_pdev)
		put_device(&cpu_pdev->dev);

	return ret;
}

static const struct of_device_id imx_dummy_dt_ids[] = {
	{ .compatible = "fsl,imx-audio-dummy", },
	{ },
};
MODULE_DEVICE_TABLE(of, imx_dummy_dt_ids);

static struct platform_driver imx_dummy_driver = {
	.driver = {
		.name = "imx-dummy",
		.pm = &snd_soc_pm_ops,
		.of_match_table = imx_dummy_dt_ids,
	},
	.probe = imx_dummy_probe,
};
module_platform_driver(imx_dummy_driver);

MODULE_DESCRIPTION("NXP i.MX DUMMY ASoC machine driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:imx-dummy");
