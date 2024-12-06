
#ifndef RW_I2C_H
#define RW_I2C_H

U32 getACK(ULONG* blockmem, int busnum, int devadr);
ULONG readI2C(ULONG* blockmem, int busnum, int devadr, UCHAR* buf, USHORT offset, int length, int eeprom16kb);
ULONG writeI2C(ULONG* blockmem, int busnum, int devadr, UCHAR* buf, USHORT offset, int length, int eeprom16kb);

#endif //RW_I2C_H