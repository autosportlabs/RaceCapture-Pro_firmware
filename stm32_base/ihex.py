#!/usr/bin/python

# Intel Hex parsing and handling library
# Copyright 2014 Jeff Ciesielski <jeff@autosportlabs.com>

import optparse

ROWTYPE_DATA = 0x00
ROWTYPE_EOF = 0x01
ROWTYPE_EXT_SEG_ADDR = 0x02
ROWTYPE_START_SEG_ADDR = 0x03
ROWTYPE_EXT_LIN_ADDR = 0x04
ROWTYPE_START_LIN_ADDR = 0x05

def chunks(l, n):
    """ Yield successive n-sized chunks from l.
    """
    for i in xrange(0, len(l), n):
        yield l[i:i+n]

class iHexRow(object):
    def __init__(self):
        self._data = None
        self.record_type = None
        self.offset = 0
        self.bytecount = 0
        self.bytelist = []
        self.checksum = 0

    def _validate(self):
        chk = self._calc_checksum()

        self_chk = ''.join(str(self)[-3:-1])
        if not chk == int(self_chk, 16):
            raise Exception("Failed to validate iHexRow:\n"+\
                            "-{}\n With checksum: {} | {}".\
                            format(str(self), chk, self_chk))

    def _parse(self):
        if not self._data[0] == ':':
            raise Exception("Invalid row start")

        self.bytecount = int(''.join(self._data[1:3]), 16)
        self.offset = int(''.join(self._data[3:7]), 16)
        self.record_type = int(''.join(self._data[7:9]), 16)
        self.bytelist = [int(x, 16) for x in chunks(self._data[9:-2], 2)]
        self.checksum = int(self._data[-2:], 16)
        self._validate()

    def _calc_checksum(self):
        checksum = sum([self.bytecount,
                        self.record_type,
                        self.offset >> 8,
                        self.offset&0xff] + self.bytelist)

        #Calculate the two's compliment of the last byte of the sum of
        #all of the bytes in the data row
        checksum = ((abs(checksum) ^ 0xff) + 1) & 0xff

        return checksum

    def from_str(self, string):
        self._data = string.strip()
        self._parse()
        return self

    def from_bytes(self, bytelist, offset, record_type):
        self.bytecount = len(bytelist)
        self.bytelist = bytelist
        self.record_type = record_type
        self.offset = offset

        self.checksum = self._calc_checksum()

        self._validate()
        
        return self

    def __str__(self):
        st = ':'
        st += "%02x"%self.bytecount
        st += "%04x"%self.offset
        st += "%02x"%self.record_type
        st += ''.join(["%02x"%x for x in self.bytelist])
        st += "%02x"%self.checksum
        st += '\n'
        return st.upper()

    def get_binary(self):
        return bytearray(self.bytelist)
        

class iHexAddrGroup(object):
    def __init__(self, addr_row):
        self.addr_row = addr_row
        self.data_rows = []

    def add_data_row(self, row):
        if type(row) == 'str':
            row = iHexRow().from_str(row)

        self.data_rows.append(row)

    def __str__(self):
        return str(self.addr_row) + ''.join([str(x) for x in self.data_rows])

    def get_binary(self):
        ba = bytearray()
        for x in self.data_rows:
            ba.extend(x.get_binary())
        return ba


class iHex(object):
    def __init__(self):
        self._addr_groups = []

    def load_ihex(self, path):
        lines = open(path, 'rb').readlines()

        #Walk through each line
        for l in lines:
            #Grab the row and convert it to an iHexRow
            row = iHexRow().from_str(l)
            if row.record_type == ROWTYPE_EXT_LIN_ADDR:
                ag = iHexAddrGroup(row)
                self._addr_groups.append(ag)
            else:
                self._addr_groups[-1].add_data_row(row)

    def __str__(self):
        return ''.join([str(x) for x in self._addr_groups])

    def get_binary(self):
        ba = bytearray()
        for x in self._addr_groups:
            ba.extend(x.get_binary())
        return ba


    def _process_binary(self, bytestring, base_offset):
        #Create variables to hold the current offset and base offset
        offset = 0x10000
        b_offset = base_offset >> 16

        #While there is data left to process
        while len(bytestring):
            #Figure out how much more space we have left in this
            #address space
            space_rem = 0x10000 - offset

            #If we don't have any space remaining in this address
            #space, create a new address space, reset the offset, and
            #increment the base offset
            if space_rem == 0:
                addr_row = iHexRow().from_bytes(bytelist=[b_offset >> 8, b_offset & 0xff],
                                                offset=0,
                                                record_type = ROWTYPE_EXT_LIN_ADDR)
                addr_group = iHexAddrGroup(addr_row)
                self._addr_groups.append(addr_group)
                offset = 0
                b_offset += 0x01
                continue

            #Grab our working data chunk and adjust the remaining data accordingly
            data = [ord(x) for x in bytestring[:16]]
            bytestring = bytestring[16:]

            #Create our iHexRow data
            drow =  iHexRow().from_bytes(bytelist=data,
                                         offset=offset,
                                         record_type=ROWTYPE_DATA)
            #Append it to the last known address group
            self._addr_groups[-1].add_data_row(drow)

            #Increment our offset by the row size we just delt with
            offset += 16

            #onto the next row
            continue
                        
        #Finally, append a row signifying the end of the file
        end = iHexRow().from_bytes(bytelist=[],
                                   offset=0,
                                   record_type = ROWTYPE_EOF)
        self._addr_groups[-1].add_data_row(end)
        
        
    def load_bytes(self, bytestring, base_offset):
        self._process_binary(bytestring, base_offset)
        

    def load_bin(self, path, base_offset):
        bindata = open(path, 'rb').read()
        self._process_binary(bindata, base_offset)


    def save_bin(self, path):
        with open(path, 'wb') as fil:
            fil.write(self.get_binary())

    def save_ihex(self, path):
        with open(path, 'wb') as fil:
            fil.write(str(self))

if __name__ == '__main__':
    ih = iHex()
    ih.load_ihex('main.ihex')
    ih.save_ihex('main2.ihex')
    ih.save_bin('main2.bin')
    ih2 = iHex()
    ih2.load_bin('main.bin', 0x08020000)
    ih2.save_ihex('main3.ihex')
    ih2.save_bin('main3.bin')

