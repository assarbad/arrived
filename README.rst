=============
 ``arrived``
=============
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
`my website`_ or `SourceForge`_. Use an archiver to unpack the archive,
I recommend `7-Zip`_, and then place the ``.exe`` files into a folder
listed in your ``PATH`` environment variable.

Build instructions
------------------
In order to build the tool yourself, you'll either need the Windows 2003
Server DDK or Visual Studio. If you use the DDK, you can simply rely on
the ``relbuild.cmd`` script, but you may have to set the ``WNETBASE``
environment variable to a sensible value (e.g. ``WNETBASE=C:\WINDDK\3790.1830``)
to match the path to the installed DDK.

If you prefer to use Visual Studio, a solution/project for Visual Studio
2005 is included and you can generate any of the other supported projects
yourself using the `premake4`_ that comes with the WinDirStat source code.
Use a tool such as ``sigcheck`` to verify the code signature on it, though.

The way you generate the projects is by calling ``premake4`` one of the
following ways::

    premake4 vs2005
    premake4 vs2008
    premake4 vs2010
    premake4 vs2012
    premake4 vs2013

The earlier Visual Studio versions may work, but no guarantees.

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
Parts written by me as a private person are in the PUBLIC DOMAIN/CC0,
whereas the utility class from FRISK Software International can be used
under the terms of the BSD license. See the license terms at the top of
the ``common/SimpleBuffer.h`` file.

.. _my website: https://assarbad.net/stuff/arrived.zip
.. _premake4: https://sourceforge.net/projects/premake4-wds/files/
.. _SourceForge: https://sourceforge.net/projects/arrived/files/
.. _7-Zip: http://7-zip.org/
