#include	<stdio.h>
#include	<fcntl.h>
#include	<linux/input.h>
#include	<linux/uinput.h>
#include	<string.h>
#include	<unistd.h>

int main(void)
{
	int infd	= open("/dev/input/by-path/platform-joypad-event-joystick", O_RDONLY);
	int outfd	= open("/dev/uinput", O_WRONLY | O_NONBLOCK);

	int rd;
	struct input_event inev[32];
	struct input_event outev[32];

	struct uinput_setup usetup;

	long keys;

	ioctl(outfd, UI_SET_EVBIT, EV_KEY);

	//setup mouse
	ioctl(outfd, UI_SET_KEYBIT, BTN_LEFT);
	ioctl(outfd, UI_SET_KEYBIT, BTN_RIGHT);

	ioctl(outfd, UI_SET_EVBIT, EV_REL);
	ioctl(outfd, UI_SET_RELBIT, REL_X);
	ioctl(outfd, UI_SET_RELBIT, REL_Y);

	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0x1234;
	usetup.id.product = 0x5678;
	strcpy(usetup.name, "DTG Keyboard & Mouse");

	ioctl(outfd, UI_DEV_SETUP, &usetup);
	ioctl(outfd, UI_DEV_CREATE);

	for (;;)
	{
		rd = read(infd, inev, sizeof(struct input_event) * 32);
		if(rd > 0)
		{
			keys = 0;
			for(int i = 0; i < rd / sizeof(struct input_event); i++)
			{
				if(inev[i].type == EV_KEY)
				{
					switch(inev[i].code)
					{
						case BTN_DPAD_LEFT:
							keys |= 1 * inev[i].value;
							break;
						case BTN_DPAD_RIGHT:
							keys |= 2 * inev[i].value;
							break;
						case BTN_DPAD_UP:
							keys |= 4 * inev[i].value;
							break;
						case BTN_DPAD_DOWN:
							keys |= 8 * inev[i].value;
							break;
						case BTN_TL:
							keys |= 16 * inev[i].value;
							break;
						case BTN_TR:
							keys |= 32 * inev[i].value;
							break;
					}
				}
			}
			outev[0].type = EV_REL;
			outev[0].code = REL_X;
			outev[0].value = !!(keys & 1) * -10;

			outev[1].type = EV_REL;
			outev[1].code = REL_X;
			outev[1].value = !!(keys & 2) * 10;

			outev[2].type = EV_REL;
			outev[2].code = REL_Y;
			outev[2].value = !!(keys & 4) * -10;

			outev[3].type = EV_REL;
			outev[3].code = REL_Y;
			outev[3].value = !!(keys & 8) * 10;

			outev[4].type = EV_KEY;
			outev[4].code = BTN_LEFT;
			outev[4].value = !!(keys & 16);

			outev[5].type = EV_KEY;
			outev[5].code = BTN_RIGHT;
			outev[5].value = !!(keys & 32);

			outev[6].type = EV_SYN;
			outev[6].code = SYN_REPORT;
			outev[6].value = 0;

			write(outfd, &outev, sizeof outev);
		}
	}
	return 0;
}
