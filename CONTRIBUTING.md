How to contribute
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

We use the Linux Kernel coding standard, which can be found here:
https://www.kernel.org/doc/Documentation/CodingStyle

We do however ask for the following modification to be made to your
source contributions:  Single line if/for/while statements should
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
