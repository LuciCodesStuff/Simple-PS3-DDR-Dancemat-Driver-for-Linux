#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <asm/unaligned.h>
#include <asm/byteorder.h>

#include <linux/hid.h>

/*

Simple PS3 DDR Dancemat Driver for Linux
(Contact info here)
(github link here)
(license here)
I was able to learn a lot about this process using the following refernces.
https://embetronicx.com/tutorials/linux/device-drivers/usb-device-driver-example/
https://github.com/Embetronicx/Tutorials/blob/master/Linux/Device_Driver/usb_device_driver/usb_driver.c
https://kernel.readthedocs.io/en/sphinx-samples/writing_usb_driver.html
https://www.nirenjan.com/2020/linux-hid-driver/ <== REAL GOAT RIGHT HERE. Nirenjan, I owe you a beer.
A big thank you to everyone supporting FOSS and Linux!

/!\
Please be aware that I have no business making Kernel modules.
If there is anything in this code that can cause harm to your system,
                    it's your fault for trusting me.
    You have been warned - I don't know a struct from an object.
/!\

BIG WARNING!
I may change the mappings! I started with the lowest bits and worked my way up.
Future versions may require you to remap your games!
I'm going to evaluate how the xbox mat driver maps hits and mimic it. Ideally I want
this to work out of the box with the least amount of end user configuration,
otherwise I've failed you, the person reading my stupid comments.

TODO:
Clean up
Confirm dependancies - too many includes
Test against major distros

CHANGELOG:
2021-10-22 - Initial release following code cleanup

TIPS:
I don't want them. This is like 20 lines of stackoverflow code.
https://my.fsf.org/donate <== They could use it!
*/

#define HID_VENDOR_ID       ( 0x1ccf )
#define HID_PRODUCT_ID      ( 0x1010 )
#define NUMBER_OF_BUTTONS = 11

static struct hid_driver hid_spsdm; // [S]imple [P]lay[S]tation [D]ance[M]at driver. Find/Replace if you don't like it.

// Kept if needed in the future.
/*
static int __check_hid_spsdm(struct device_driver *drv, void *data)
{
	struct hid_driver *hdrv = to_hid_driver(drv);
	struct hid_device *hdev = data;

	if (hdrv == &hid_spsdm)
		return 0;

	return hid_match_device(hdev, hdrv) != NULL;
}
*/

static bool hid_spsdm_match(struct hid_device *hdev,
			      bool ignore_special_driver)
{
	if (ignore_special_driver)
		return true;

	if (hdev->quirks & HID_QUIRK_HAVE_SPECIAL_DRIVER)
		return false;

	return true;
}

static int hid_spsdm_probe(struct hid_device *hdev,
			     const struct hid_device_id *id)
{
	int ret;

	hdev->quirks |= HID_QUIRK_INPUT_PER_APP;

	ret = hid_parse(hdev);
	if (ret)
		return ret;

	return hid_hw_start(hdev, HID_CONNECT_DEFAULT);
}

static int hid_spsdm_input_mapping(struct hid_device *dev,
                             struct hid_input *input,
                             struct hid_field *field,
                             struct hid_usage *usage,
                             unsigned long **bit,
                             int *max)
{
    return -1;
}

static int hid_spsdm_input_configured(struct hid_device *dev,
                                struct hid_input *input)
{
    struct input_dev * input_dev = input->input;
    int i;

    hid_set_drvdata(dev, input_dev);

    set_bit(EV_KEY, input_dev->evbit);
    //set_bit(EV_ABS, input_dev->evbit); // Eventually I want the analog and digital hits to return -- for completion sake.

    for (i = 0; i < NUMBER_OF_BUTTONS; i++) {
        set_bit(BTN_TRIGGER_HAPPY1 + i, input_dev->keybit);
        // Using BTN_TRIGGER_HAPPY here instead of BTN_TRIGGER. This seems to get better results, but shouldn't?
        // It might have been because my original attempts mapped 24 buttons, most were empty for debugging/learning.
        // If there are any issues with using BTN_TRIGGER_HAPPY please let me know and I will update. Otherwise it ain't broke.
    }

    return 0;
}

static int hid_spsdm_raw_event(struct hid_device *dev,
                         struct hid_report *report, u8 *data, int len)
{
    struct input_dev *input_dev = hid_get_drvdata(dev);
    int ret; // Not used?
    //int idx; // not used - no longer looping indexes
    //int btn; // not used - no longer looping buttons
    //int i; // not used - no longer looping... loops

    /*
    Indexes:
    [0] holds button presses
    [1] holds extra keys
    [3] holds dpad movements, but I am unable to parse this correctly at this time
        This data seems to be locked at 0x08 when not in use. It appears the dpad digital hits go low, where as the btn hits go high
        Once I get this sorted out I will change this over.
    [7] - [10] are anaglog returns of the "left stick", which can be used in place of the dpad for our uses.
    Note that the pad will return seperate analog data for each direction, so left and right, or up and down work.
    I plan on using the dpad return once I figure it out, not because the current method doesn't work but because I believe
    it's the correct way to make this driver. Mapping the analog to digital is really a hack.
    __I refuse to make this a loop. Fight me.__
    */
    input_report_key(input_dev, BTN_TRIGGER_HAPPY1, data[0] & 0x01); // SQUARE
    input_report_key(input_dev, BTN_TRIGGER_HAPPY2, data[0] & 0x02); // X
    input_report_key(input_dev, BTN_TRIGGER_HAPPY3, data[0] & 0x04); // O
    input_report_key(input_dev, BTN_TRIGGER_HAPPY4, data[0] & 0x08); // TRIANGLE
    input_report_key(input_dev, BTN_TRIGGER_HAPPY5, data[1] & 0x01); // SELECT
    input_report_key(input_dev, BTN_TRIGGER_HAPPY6, data[1] & 0x02); // START
    input_report_key(input_dev, BTN_TRIGGER_HAPPY7, data[1] & 0x10); // PS Button
    input_report_key(input_dev, BTN_TRIGGER_HAPPY8, data[7] & 0xFF); // RIGHT
    input_report_key(input_dev, BTN_TRIGGER_HAPPY9, data[8] & 0xFF); // LEFT
    input_report_key(input_dev, BTN_TRIGGER_HAPPY10, data[9] & 0xFF); // UP
    input_report_key(input_dev, BTN_TRIGGER_HAPPY11, data[10] & 0xFF); // DOWN
    input_sync(input_dev);

    return ret; // ret = NULL? wtf? I think I got some reading to do.... fml....
}

static const struct hid_device_id hid_table[] = {
	{ HID_USB_DEVICE(HID_VENDOR_ID, HID_PRODUCT_ID) },
	{ }
};
MODULE_DEVICE_TABLE(hid, hid_table);

static struct hid_driver hid_spsdm = {
	.name = "hid-spsdm",
	.id_table = hid_table,
	.match = hid_spsdm_match,
	.probe = hid_spsdm_probe,
    .input_mapping = hid_spsdm_input_mapping,
    .input_configured = hid_spsdm_input_configured,
    .raw_event = hid_spsdm_raw_event,
};
module_hid_driver(hid_spsdm);

MODULE_AUTHOR("NO");
MODULE_DESCRIPTION("NO");
MODULE_LICENSE("GPL");