How to Contribute
=================

Being an open hardware company, we love contributions from outside our
organization.   If you would like to contribute, please do the
following:

1. Fork our repository on Github
2. Make a feature branch and commit your changes there
3. Issue a pull request against the master branch
4. Once you've issued a request, we'll review your changes and merge
   them once we've had a chance to test.


Coding Standard
===============

# Standard

We use the Linux Kernel coding standard, which can be found here:
https://www.kernel.org/doc/Documentation/CodingStyle

# Modifications

While we think the Linux standard is great, we have our own tweaks
that we have made.  They are as follows...

## File Headers

Always ensure that the file header at the top of the file matches the
following:

```C
/*
 * Race Capture Firmware
 *
 * Copyright (C) 2015 Autosport Labs
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

## Includes

Please sort all include alphabetically.  Not only does this make things
easier to find, it also eliminates duplicates.

## Control Statements Always Have Brackets

Single line if/for/while statements should
always include braces.

So:

```C
if (foo)
	bar();
```

should be:

```C
if (foo) {
	bar();
}
```
