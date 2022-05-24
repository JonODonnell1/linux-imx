// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * ASoC dummy codec driver
 * Copyright 2022 Pierluigi Passaro <pierluigi.p@variscite.com>
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>

#define DUMMY_CODEC_CHANNEL_MAX	384
#define DUMMY_CODEC_RATES		SNDRV_PCM_RATE_8000_384000
#define DUMMY_CODEC_FORMATS \
	(SNDRV_PCM_FMTBIT_S8 | \
	SNDRV_PCM_FMTBIT_U8 | \
	SNDRV_PCM_FMTBIT_S16_LE | \
	SNDRV_PCM_FMTBIT_U16_LE | \
	SNDRV_PCM_FMTBIT_S24_LE | \
	SNDRV_PCM_FMTBIT_S24_3LE | \
	SNDRV_PCM_FMTBIT_U24_LE | \
	SNDRV_PCM_FMTBIT_S32_LE | \
	SNDRV_PCM_FMTBIT_U32_LE | \
	SNDRV_PCM_FMTBIT_DSD_U8 | \
	SNDRV_PCM_FMTBIT_DSD_U16_LE | \
	SNDRV_PCM_FMTBIT_DSD_U32_LE | \
	SNDRV_PCM_FMTBIT_IEC958_SUBFRAME_LE)

static int dummy_codec_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	dev_dbg(dai->dev, "%s\n", __func__);
	return 0;
}

static int dummy_codec_hw_params(struct snd_pcm_substream *substream,
				  struct snd_pcm_hw_params *params,
				  struct snd_soc_dai *dai)
{
	dev_dbg(dai->dev, "%s: bclk %d, channels %u, rate %u, width %d\n", __func__,
		snd_soc_params_to_bclk(params),
		params_channels(params), params_rate(params), params_width(params));
	return 0;
}

static int dummy_codec_mute(struct snd_soc_dai *dai, int mute, int direction)
{
	dev_dbg(dai->dev, "%s: mute %d, dir %d\n", __func__, mute, direction);
	return 0;
}

static int dummy_codec_set_dai_fmt(struct snd_soc_dai *dai, unsigned int fmt)
{
	dev_dbg(dai->dev, "%s: fmt %u\n", __func__, fmt);
	return 0;
}

static int dummy_codec_set_pll(struct snd_soc_dai *dai, int pll_id, int source,
				unsigned int freq_in, unsigned int freq_out)
{
	dev_dbg(dai->dev, "%s: pll %d, src %d, f_in %u, f_out %u\n", __func__,
		pll_id, source, freq_in, freq_out);
	return 0;
}

static int dummy_codec_set_dai_sysclk(struct snd_soc_dai *dai, int clk_id,
				       unsigned int freq, int dir)
{
	dev_dbg(dai->dev, "%s: clk %d, freq %u, dir %d\n", __func__,
		clk_id, freq, dir);
	return 0;
}

static int dummy_codec_set_tdm_slot(struct snd_soc_dai *dai, unsigned int tx_mask,
				     unsigned int rx_mask, int slots, int slot_width)
{
	dev_dbg(dai->dev, "%s: txmask %u, rxmask %u, slots %d, width %d\n", __func__,
		tx_mask, rx_mask, slots, slot_width);
	return 0;
}

static const struct snd_soc_dai_ops dummy_codec_dai_ops = {
	.startup	= dummy_codec_startup,
	.hw_params	= dummy_codec_hw_params,
	.mute_stream	= dummy_codec_mute,
	.set_fmt	= dummy_codec_set_dai_fmt,
	.set_pll	= dummy_codec_set_pll,
	.set_sysclk	= dummy_codec_set_dai_sysclk,
	.set_tdm_slot	= dummy_codec_set_tdm_slot,
	.no_capture_mute = 1,
};

static struct snd_soc_dai_driver dummy_codec_dai = {
	.name = "dummy-codec-dai",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = DUMMY_CODEC_CHANNEL_MAX,
		.rates = DUMMY_CODEC_RATES,
		.formats = DUMMY_CODEC_FORMATS,
	},
	.capture = {
		 .stream_name = "Capture",
		.channels_min = 1,
		.channels_max = DUMMY_CODEC_CHANNEL_MAX,
		.rates = DUMMY_CODEC_RATES,
		.formats = DUMMY_CODEC_FORMATS,
	},
	.ops = &dummy_codec_dai_ops,
};

static const struct snd_soc_component_driver soc_component_dev_dummy_codec = {
	.idle_bias_on		= 1,
	.use_pmdown_time	= 1,
	.endianness		= 1,
	.non_legacy_dai_naming	= 1,
};

static int dummy_codec_probe(struct platform_device *pdev)
{
	return devm_snd_soc_register_component(&pdev->dev,
				      &soc_component_dev_dummy_codec,
				      &dummy_codec_dai, 1);
}

static int dummy_codec_remove(struct platform_device *pdev)
{
	return 0;
}

static const struct platform_device_id dummy_codec_driver_ids[] = {
	{ .name = "dummy-codec", },
	{},
};
MODULE_DEVICE_TABLE(platform, dummy_codec_driver_ids);

#if defined(CONFIG_OF)
static const struct of_device_id dummy_codec_codec_of_match[] = {
	{ .compatible = "linux,dummy-codec", },
	{},
};
MODULE_DEVICE_TABLE(of, dummy_codec_codec_of_match);
#endif

static struct platform_driver dummy_codec_driver = {
	.driver = {
		.name = "dummy-codec",
		.of_match_table = of_match_ptr(dummy_codec_codec_of_match),
	},
	.probe = dummy_codec_probe,
	.remove = dummy_codec_remove,
	.id_table = dummy_codec_driver_ids,
};

module_platform_driver(dummy_codec_driver);

MODULE_AUTHOR("Pierluigi Passaro <pierluigi.p@variscite.com>");
MODULE_DESCRIPTION("ASoC dummy codec driver");
MODULE_LICENSE("GPL");
