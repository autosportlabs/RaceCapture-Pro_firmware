Coding Standard
===============
This documents should give you the skinny on how we format our
code here at AutosportLabs.  This applies only to this project and
may be different on other AutosportLabs projects.

# Standard

We use the [Linux Kernel coding
standard](https://www.kernel.org/doc/Documentation/CodingStyle) as the
base of our standard.  In our experience, it leads to some of the best
and most maintainable code.  We do however have some modifications
that we make to it that are outlined below.  If it isn't listed as a
modification below, then default to the standard.

## Modifications

While we think the Linux standard is great, we have our own tweaks
that we have made.  They are as follows...

### File Headers

Always ensure that the file header at the top of the file matches the
following:

```C
/*
 * Race Capture Firmware
 *
 * Copyright (C) 2016 Autosport Labs
 *
 * This file is part of the Race Capture firmware suite
 *
 * This is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the GNU General Public License for more details. You should
 * have received a copy of the GNU General Public License along with
 * this code. If not, see <http://www.gnu.org/licenses/>.
 */
```

If it doesn't, please fix it as part of your patch.

### Includes

Please sort all include alphabetically.  Not only does this make things
easier to find, it also eliminates duplicates.

## Reminders
These are common mistakes people make, please don't make them

* Use C-89 comment standards.  Yes we use C99 for our code.  But we use
  C-89 coding standards.  Use them and only them.
* Use tabs, not spaces.  And set your tab width to 8 spaces.