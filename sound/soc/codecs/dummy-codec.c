// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * ASoC dummy codec driver
 * Copyright 2022 Pierluigi Passaro <pierluigi.p@variscite.com>
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <sound/soc.h>

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
