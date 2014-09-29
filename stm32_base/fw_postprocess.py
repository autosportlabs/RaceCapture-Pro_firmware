#!/usr/bin/python

# Postprocessing script for Racecapture MK2 Firmware
# Copyright 2014 Jeff Ciesielski <jeff@autosportlabs.com>

import crcmod
import struct
import optparse
import subprocess
from ihex import ihex

#Size of the info block in bytes
INFO_SIZE = 20
INFO_MAGIC = 0xDEADFA11

class FwImage(object):
    def __init__(self, bin_path, image_start):
        self._base_offset = image_start
        self._bin = open(bin_path, 'rb').read()
        self._app_len = len(self._bin)

        self._split_image()
        self._gen_crcs()
        self._reassemble()

    def save_upgrade_img(self, path="image.ihex"):
        """
        Outputs the CRC'd image as an intel hex file
        """
        ih = ihex.iHex()
        ih.load_bytes(self._bin, self._base_offset)
        ih.save_ihex(path)

    def save_binary_img(self, bin_path):
        """
        Saves the CRC'd image as a binary file
        """
        with open(bin_path, 'wb') as fil:
            fil.write(self._bin)

    def _reassemble(self):
        """
        Reassembles the individual firmware blocks into a contiguous block
        """
        self._bin = self._pre_info + self._info + self._post_info
        print "Application length: {}".format(len(self._bin))

    def _crc_block(self, block):
        """
        CRC's a block of 32 bit words

        This is necessary as the STM32F4's CRC engine operates 32 bits at a time
        """
        crc32 = crcmod.Crc(0x104c11db7, initCrc=0xffffffff, rev=False)
        for idx in range(0, len(block), 4):
            tmp = struct.unpack("<I", block[idx:idx+4])[0]
            crc32.update(struct.pack(">I", tmp))

        return crc32.crcValue

    def _gen_crcs(self):
        """
        Generates the CRCs for the image and info block
        """
        app_crc = self._crc_block(self._pre_info+self._post_info)

        #Create the app info struct (sans info CRC)
        tmp_info = struct.pack("<IIII", INFO_MAGIC, self._base_offset,
                               self._app_len, app_crc)

        #Generate the info block crc
        info_crc = self._crc_block(tmp_info)

        #Now create the final info block
        self._info = struct.pack("<IIIII", INFO_MAGIC, self._base_offset,
                                 self._app_len, app_crc, info_crc)

        print "Application CRC: {}".format(hex(app_crc))
        print "Info CRC: {}".format(hex(info_crc))

    def _scan_for_info(self):
        """
        Returns the starting index of the application info block
        """

        #Call the 'nm' command to obtain a list of all of the symbols
        #in the firmware
        cmd = [ 'arm-none-eabi-nm', 'main.elf']
        output = subprocess.Popen( cmd, stdout=subprocess.PIPE ).communicate()[0].split('\n')

        #Now, loop through this symbol list and try to find the info block
        for var in output:
            try:
                offset, stype, name = var.split(' ')
                if name == 'info_block':
                    offset = int(offset, 16) - self._base_offset
                    return offset
            except:
                #Some symbols only have 2 entries (no offset), so we
                #just skip
                pass

        #If we never returned the offset, obviously there is a problem
        raise Exception("Unable to locate info block")

    def _split_image(self):
        """
        Splits the image into pre_info and post_info blobs
        (also stores the info block even though we're going to mutilate it)
        """
        info_offset = self._scan_for_info()
        print "Found info block @ offset: {}".format(hex(info_offset))
        self._pre_info = self._bin[:info_offset]
        self._info = self._bin[info_offset:info_offset + INFO_SIZE]
        self._post_info = self._bin[info_offset + INFO_SIZE:]

if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option('-f', '--filename',
                      dest="input_filename",
                      help="Filename of .bin file to process")

    parser.add_option('-b', '--bin',
                      dest="binary_filename",
                      help="Generate a CRC'd binary file")

    parser.add_option('-i', '--ihex',
                      dest="ihex_filename",
                      help="Generate a CRC'd iHex file")

    parser.add_option('-o', '--offset',
                      dest="base_offset",
                      type="int",
                      default=0x00000000,
                      help="Base offset of the image")

    options, remainder = parser.parse_args()

    if not options.input_filename:
        parser.error("No filename specified")

    if not options.binary_filename and not options.ihex_filename:
        parser.error("Must specifiy a binary and/or ihex file to store")

    img = FwImage(options.input_filename, options.base_offset)

    if options.binary_filename:
        img.save_binary_img(options.binary_filename)

    if options.ihex_filename:
        img.save_upgrade_img(options.ihex_filename)
