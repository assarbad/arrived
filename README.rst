``arrived``
===========
:Author: Oliver Schneider


About
-----
``arrived`` is a little Windows tool that will invoke a command whenever
a new USB device, or more specifically *drive*, arrives in the system.
That is, whenever you plug in or remove a USB drive such as a USB key
or an external hard drive. So now you also know where the name came from.

Installation
------------
No installation required. Download the binaries (32 and 64bit) from
`my website`_.

Requirements
------------
The program should run on Windows 2000 and later. Itanium CPUs are not
supported, but feel free to build that yourself, if needed.

Usage
-----

Syntax::

    arrived <command> [-only=drive_type,...]

The default if ``--only`` is not explicitly given is ``removable``.
Otherwise the following drive types should be valid:

* ``unknown``
* ``noroot``
* ``removable``
* ``fixed``
* ``remote``
* ``cdrom``
* ``ramdisk``

License
-------
Parts written by me as a private person are in the PUBLIC DOMAIN (CC0),
whereas the utility class from FRISK Software International can be used
under the terms of the BSD license. See the license terms at the top of
the ``common/SimpleBuffer.h`` file.

.. _my website: https://assarbad.net/stuff/arrived.zip
