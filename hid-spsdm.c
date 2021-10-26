#include <linux/module.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <asm/unaligned.h>
#include <asm/byteorder.h>

#include <linux/hid.h>

/*
Simple PS3 DDR Dancemat Driver for Linux
https://github.com/UntrustedRoot/Simple-PS3-DDR-Dancemat-Driver-for-Linux

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

TODO:
INPROGRESS-Confirm dependancies - too many includes
TOOLAZY-Test against major distros

CHANGELOG:
2021-10-22 - 	Initial release following code cleanup
2021-10-25 - 	Changed button mappings to use dpad, cardinal, and start/select/mode
		No longer using analog for dpad

TIPS:
I don't want them. This is like 20 lines of stackoverflow code.
https://my.fsf.org/donate <== They could use it!
*/

#define HID_VENDOR_ID       ( 0x1ccf )
#define HID_PRODUCT_ID      ( 0x1010 )
//#define NUMBER_OF_BUTTONS   11

static struct hid_driver hid_spsdm; // [S]imple [P]lay[S]tation [D]ance[M]at driver. Find/Replace if you don't like it.

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

    hid_set_drvdata(dev, input_dev);

    set_bit(EV_KEY, input_dev->evbit);
    //set_bit(EV_ABS, input_dev->evbit); // Eventually I want the analog and digital hits to return -- for completion sake.

    // Set the dpad and cardinal bits
    set_bit(BTN_DPAD_UP, input_dev->keybit);
    set_bit(BTN_DPAD_DOWN, input_dev->keybit);
    set_bit(BTN_DPAD_LEFT, input_dev->keybit);
    set_bit(BTN_DPAD_RIGHT, input_dev->keybit);
    set_bit(BTN_NORTH, input_dev->keybit);      // TRIANGLE
    set_bit(BTN_SOUTH, input_dev->keybit);      // X
    set_bit(BTN_EAST, input_dev->keybit);       // CIRCLE
    set_bit(BTN_WEST, input_dev->keybit);       // SQUARE
    set_bit(BTN_START, input_dev->keybit);
    set_bit(BTN_SELECT, input_dev->keybit);
    set_bit(BTN_MODE, input_dev->keybit);       // PS Button

    return 0;
}

static int hid_spsdm_raw_event(struct hid_device *dev,
                         struct hid_report *report, u8 *data, int len)
{
    struct input_dev *input_dev = hid_get_drvdata(dev);
    //int ret;

    /*
    Indexes:
    [0] holds button presses
    [1] holds extra keys
    [2] holds dpad movements
	unlike the button hits, this returns 0x08 and goes low when the dpad is press.
	subtract this value from 0x08 and parse like a button.
    [7] - [10] are anaglog returns of the "left stick", which can be used in place of the dpad for our uses.
    */

    //Correct mappings AFAIK
    input_report_key(input_dev, BTN_WEST, data[0] & 0x01);      // SQUARE
    input_report_key(input_dev, BTN_SOUTH, data[0] & 0x02);     // X
    input_report_key(input_dev, BTN_EAST, data[0] & 0x04);      // CIRCLE
    input_report_key(input_dev, BTN_NORTH, data[0] & 0x08);     // TRIANGLE
    input_report_key(input_dev, BTN_SELECT, data[1] & 0x01);    // SELECT
    input_report_key(input_dev, BTN_START, data[1] & 0x02);     // START
    input_report_key(input_dev, BTN_MODE, data[1] & 0x10);      // PS Button

    //Former analog to digital mappings. Works, but not right!
    input_report_key(input_dev, BTN_DPAD_RIGHT, data[7] & 0xFF);	// RIGHT
    input_report_key(input_dev, BTN_DPAD_LEFT, data[8] & 0xFF);		// LEFT
    input_report_key(input_dev, BTN_DPAD_UP, data[9] & 0xFF);		// UP
    input_report_key(input_dev, BTN_DPAD_DOWN, data[10] & 0xFF);	// DOWN

    //To many issues trying to get the bits right on these attempts. Keep trying....
    //input_report_key(input_dev, BTN_DPAD_RIGHT, (0x08 - data[2]) & 0x06);    // RIGHT
    //input_report_key(input_dev, BTN_DPAD_LEFT, (0x08 - data[2]) & 0x02);     // LEFT
    //input_report_key(input_dev, BTN_DPAD_UP, (0x08 - data[2]) & 0x08);       // UP
    //input_report_key(input_dev, BTN_DPAD_DOWN, (0x08 - data[2]) & 0x04);     // DOWN

    input_sync(input_dev);

    //return ret;
    return 0; // Meh
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

MODULE_AUTHOR("UntrustedRoot");
MODULE_DESCRIPTION("Simple PS3 DDR Dancemat Driver for Linux");
MODULE_LICENSE("GPL");
