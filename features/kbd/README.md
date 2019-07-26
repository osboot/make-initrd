# Feature: kbd

Feature adds keymap and console fonts to initramfs.

## Parameters

- **KBD_KEYMAP** -- Name or full path to console keymap;
- **KBD_BACKSPACE** -- The way how to handle backspace (possible options: Backspace, Delete);
- **KBD_GRP_TOGGLE** -- The way how to toggle modes;
- **KBD_FONT** -- Name or full path to console font;
- **KBD_FONT_UNIMAP** -- Unimap for console font (optional);
- **KBD_TTYS** -- TTY numbers to which you want to adjust the font and keymap.

System config files:

- `/etc/sysconfig/consolefont`
- `/etc/sysconfig/keyboard`

Initramfs config file:

- `/etc/sysconfig/console`
