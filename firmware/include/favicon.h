
// a) https://www.favicon-generator.org/
// b) exiftool -all:all= -r
// c) hexlify.py:
//    import sys
//    f = open (sys.argv[1], 'rb').read()
//    for n, c in enumerate(f):
//      if n % 16 == 0: print ('        "', end = '')
//      print (f"\\x{c:02x}", end = '')
//      if n % 16 == 15: print ('" \\')
//    if n % 16 != 15: print ('"')

#define FAVICON_PANEL_16 \
        "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" \
        "\x00\x00\x00\x10\x00\x00\x00\x10\x08\x03\x00\x00\x00\x28\x2d\x0f" \
        "\x53\x00\x00\x00\x04\x67\x41\x4d\x41\x00\x00\xb1\x8f\x0b\xfc\x61" \
        "\x05\x00\x00\x00\x20\x63\x48\x52\x4d\x00\x00\x7a\x26\x00\x00\x80" \
        "\x84\x00\x00\xfa\x00\x00\x00\x80\xe8\x00\x00\x75\x30\x00\x00\xea" \
        "\x60\x00\x00\x3a\x98\x00\x00\x17\x70\x9c\xba\x51\x3c\x00\x00\x01" \
        "\x9b\x50\x4c\x54\x45\xfc\xfe\xff\xff\xff\xff\xcb\xcd\xcf\x22\x25" \
        "\x30\x12\x16\x21\x11\x15\x21\x11\x15\x23\x12\x16\x25\x10\x13\x1f" \
        "\x8f\x91\x93\x9b\x9d\xa1\x14\x17\x22\x14\x18\x25\x14\x17\x23\x13" \
        "\x17\x22\x14\x17\x24\x17\x19\x27\x22\x24\x2e\xc1\xc2\xc3\xf8\xfa" \
        "\xfb\x61\x63\x6b\x11\x13\x1f\x15\x19\x24\x17\x19\x24\x18\x1a\x24" \
        "\x18\x1b\x26\x16\x19\x26\x42\x44\x4b\xe7\xe8\xe9\xfd\xff\xff\xfe" \
        "\xff\xff\xe0\xe2\xe4\x33\x36\x3f\x16\x19\x24\x16\x17\x22\x16\x18" \
        "\x22\x17\x18\x23\x11\x12\x1e\x71\x73\x76\xfb\xfc\xfd\xb5\xb7\xba" \
        "\x1a\x1c\x27\x15\x17\x22\x15\x17\x21\x14\x15\x20\x14\x16\x23\x17" \
        "\x1a\x26\x1b\x1c\x25\xaa\xac\xad\x7c\x7f\x85\x11\x13\x1d\x16\x18" \
        "\x24\x17\x19\x25\x18\x1a\x25\x30\x32\x38\xd7\xd9\xd9\xef\xf1\xf2" \
        "\x47\x49\x51\x12\x13\x1e\x17\x19\x22\x16\x17\x23\x10\x11\x1c\x59" \
        "\x5a\x5f\xf4\xf6\xf7\xcc\xce\xd0\x24\x25\x2e\x16\x18\x23\x19\x1a" \
        "\x24\x16\x17\x21\x15\x16\x21\x17\x17\x21\x15\x17\x20\x92\x93\x95" \
        "\x99\x9a\x9e\x16\x16\x20\x17\x19\x23\x1a\x1b\x25\x23\x25\x2c\xc4" \
        "\xc6\xc7\xf8\xfa\xfa\x61\x62\x68\x15\x16\x20\x11\x12\x1d\x44\x45" \
        "\x49\xea\xec\xec\xdf\xe1\xe2\x33\x35\x3b\x15\x16\x1f\x13\x14\x1d" \
        "\x78\x7a\x7c\xfc\xfd\xfe\xb2\xb4\xb7\x1a\x1c\x26\x18\x1b\x25\x1a" \
        "\x1b\x22\x19\x1b\x22\xae\xaf\xb0\x86\x88\x8b\x14\x16\x1e\x19\x1b" \
        "\x25\x17\x19\x21\x17\x19\x20\x12\x13\x1a\x2c\x2d\x32\xcc\xce\xcf" \
        "\xfb\xfe\xff\xd8\xdb\xdb\x72\x74\x76\x25\x26\x2c\x19\x1c\x24\x13" \
        "\x16\x1d\x11\x13\x18\x4c\x4d\x4f\xc1\xc3\xc3\xe2\xe4\xe6\xf6\xf8" \
        "\xf8\xb6\xb9\xb9\x4f\x51\x55\x1b\x1c\x23\x19\x1b\x23\x15\x17\x1f" \
        "\x70\x71\x72\xbb\xbb\xbc\xc3\xc4\xc5\xde\xe0\xe1\xf6\xf8\xf9\xfd" \
        "\xfe\xff\xe3\xe5\xe6\x7e\x81\x82\x21\x22\x27\x1d\x1f\x26\x94\x95" \
        "\x95\xbd\xbd\xbe\xbd\xbe\xbe\xc8\xca\xcb\xe5\xe7\xe9\xfa\xfc\xfd" \
        "\x52\x5f\xd3\xea\x00\x00\x00\x01\x62\x4b\x47\x44\x01\xff\x02\x2d" \
        "\xde\x00\x00\x00\xd2\x49\x44\x41\x54\x18\xd3\x63\x60\x00\x02\x46" \
        "\x26\x66\x16\x16\x56\x36\x76\x0e\x4e\x46\x06\x30\x60\xe4\xe2\xe6" \
        "\xe1\xe5\xe3\x17\x10\x14\x82\x0a\x08\x8b\x88\x8a\x89\x4b\x48\x4a" \
        "\x49\xcb\xc8\x82\xf9\x72\xf2\x0a\xdc\x8a\x4a\xca\x2a\xaa\x6a\xea" \
        "\x50\x1d\x1a\x9a\x5a\xda\x3a\xba\x7a\xfa\x06\x50\x1d\xb2\x86\x46" \
        "\x5a\xbc\xc6\x26\xa6\x66\xe6\x72\x10\xbe\x85\xa5\x95\xb5\x89\xa9" \
        "\x8d\xad\x9d\x3d\xc4\x08\x46\x07\x47\x27\x67\x17\x57\x37\x77\x0f" \
        "\x39\xa8\x11\x9e\x4a\xa6\x36\x5e\xde\x3e\xbe\x7e\x50\x23\xfc\x03" \
        "\x02\x55\x8c\x9d\x55\x82\x82\x43\xa0\x96\x86\x86\x85\x4b\x48\xb8" \
        "\x84\x47\x44\x46\x41\x75\x44\xc7\x78\xc7\xba\x87\xc7\xc5\x27\x40" \
        "\x75\xc8\x25\x26\x25\x7b\xa7\xa4\xa6\xa5\x67\x64\x42\x5d\x91\x95" \
        "\x9d\x93\x92\x9b\x97\x5f\x50\x58\x24\x0c\x55\x52\x5c\x52\x5a\x56" \
        "\x5e\x51\x59\x55\x5d\x53\x0b\x11\xa9\x93\xab\x6f\x68\x6c\x6a\x6e" \
        "\x69\x6d\x6b\xef\x60\x00\x00\x01\x53\x2a\x2a\x63\x34\xcd\xf7\x00" \
        "\x00\x00\x00\x49\x45\x4e\x44\xae\x42\x60\x82"

#define FAVICON_PANEL_32 \
        "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a\x00\x00\x00\x0d\x49\x48\x44\x52" \
        "\x00\x00\x00\x20\x00\x00\x00\x20\x08\x06\x00\x00\x00\x73\x7a\x7a" \
        "\xf4\x00\x00\x00\x04\x67\x41\x4d\x41\x00\x00\xb1\x8f\x0b\xfc\x61" \
        "\x05\x00\x00\x00\x20\x63\x48\x52\x4d\x00\x00\x7a\x26\x00\x00\x80" \
        "\x84\x00\x00\xfa\x00\x00\x00\x80\xe8\x00\x00\x75\x30\x00\x00\xea" \
        "\x60\x00\x00\x3a\x98\x00\x00\x17\x70\x9c\xba\x51\x3c\x00\x00\x00" \
        "\x06\x62\x4b\x47\x44\x00\xff\x00\xff\x00\xff\xa0\xbd\xa7\x93\x00" \
        "\x00\x07\x4a\x49\x44\x41\x54\x58\xc3\x9d\x97\xd9\x72\x5d\xc5\x15" \
        "\x86\xbf\xee\xde\xd3\xd9\x67\xd0\x99\x74\x34\x3b\x48\x32\x48\xe0" \
        "\x21\x60\xe1\x32\xd8\x15\x5e\x80\xdc\xf2\x00\xe4\x41\x12\x9e\x21" \
        "\x37\xc9\x43\xa4\x20\x95\xe2\x9a\x82\x0c\x10\x70\x62\x06\x0f\xc2" \
        "\x24\x15\xe2\x59\x96\xce\xbc\xe7\xb1\x73\x21\x93\x32\x20\x4b\xa7" \
        "\xe8\xaa\x7d\xb3\x2f\x7a\x7d\xf5\xff\xfd\xaf\x5e\x2d\xf2\x52\x6b" \
        "\x66\x58\x4a\xc0\x67\xff\xf8\x27\x6f\xbd\xf5\x36\xa3\xf1\x04\xad" \
        "\x0b\x84\x94\x68\xad\xc9\xf3\x1c\xc7\x76\x08\xa3\x04\xdb\xb6\xc8" \
        "\xf3\x12\x0d\xb8\x15\x9b\x28\x8c\x90\x52\xb0\xfe\xdc\x02\xbf\xff" \
        "\xdd\x6f\xb9\x72\xf9\x32\xc5\x53\x15\xe5\x2c\xc5\xbf\x5b\xdf\x7e" \
        "\x7b\x87\x34\x4b\x68\x36\xeb\x38\x8e\x4d\xb3\x59\xc7\xb6\x0d\x3a" \
        "\x9d\x16\xb6\x63\x52\x71\x6c\xea\xb5\x2a\x52\x0a\x6a\xd5\x0a\x96" \
        "\x65\x02\x1a\xc7\x36\xd9\xde\xda\x64\x6b\x6b\x9b\xf2\x07\x7b\x1a" \
        "\xb3\x16\xcf\x8a\x82\x4f\x3f\xbd\x4a\x18\x86\x34\x1a\x73\x68\x24" \
        "\x52\x1a\x08\xa1\xa8\x54\x1c\xa2\x30\xa4\xd9\xac\xd3\x9d\xef\xa0" \
        "\xb5\x66\x65\x65\x91\x30\x8c\x50\x52\xd3\x6a\xd5\xd9\xda\x7a\x81" \
        "\x56\xbb\xcd\x0f\xf5\x9e\x09\x40\x0a\x78\xb4\xf7\x98\x8f\x3f\xfe" \
        "\x8c\xa2\x28\x99\x4c\xa6\x94\x25\xe4\x59\x49\x14\x47\xc4\x51\x4c" \
        "\x92\x24\x54\x9c\x0a\xfd\xfe\x88\x3c\xcb\x89\xc2\x88\x2c\xcb\xd1" \
        "\x5a\x23\x74\xce\xf6\xf6\x36\x4a\xf0\x3d\xf9\x67\x06\x10\xc0\xd7" \
        "\xb7\xbf\xa1\x3f\x18\xb1\xb0\x30\x8f\xef\xfb\x98\xa6\x49\x96\x6b" \
        "\xda\x9d\x39\x6c\x4b\x31\x1c\x8e\xa9\xd5\xaa\x8c\xc7\x1e\xae\xeb" \
        "\x50\x71\xab\x0c\xfa\x63\x4c\x43\xb2\x79\xfa\x39\x2e\xbd\xf6\x1a" \
        "\x47\x1d\xb6\x99\x2d\xb8\x7a\xf5\x73\xee\xde\x79\x80\x6d\x9b\x64" \
        "\x79\x86\x6d\xdb\x64\x59\x86\x28\x73\x22\xa1\xa1\x2c\xb1\x2d\x03" \
        "\xd3\x54\x2c\x2e\xf6\x70\x6c\x87\xb2\x28\xa8\xd5\x1c\xce\x9f\x3b" \
        "\xc3\xda\xda\x29\x4a\xfd\x13\x00\x84\x00\xcf\x0f\xb8\x7e\xfd\x3a" \
        "\xbd\x85\x0e\x8e\x6d\x31\x99\x7a\x58\x96\xc5\x70\x38\x22\x8a\x63" \
        "\xd2\x24\xc5\xb2\x4c\x1e\xde\x7f\x44\x9c\xa4\xa4\x71\x42\x91\xe7" \
        "\x14\x65\x49\x96\xda\xac\xad\xad\xe2\x58\xe6\x8f\xe4\x9f\x09\x40" \
        "\x02\xb7\xbf\xf9\x86\x4f\x3e\xb9\x4a\x92\xc4\x68\x5d\xd2\x68\xd4" \
        "\x71\x9c\x0a\x4a\x49\x1c\xdb\xe2\xe0\x60\xc0\xe2\x52\x0f\x7f\xea" \
        "\xe3\xe4\x19\xae\x5b\xe5\xe0\x60\x88\x10\x8a\x4e\xbb\xc5\xeb\x97" \
        "\x2f\x3f\x73\xff\x99\x2c\xd8\xdd\xbd\xcd\x70\x34\x46\x6b\xcd\x74" \
        "\xea\x63\x99\x16\x79\x9e\x63\x5b\x06\x9e\x80\x2c\xcb\x99\x4e\x7d" \
        "\x92\xa4\xa0\xd9\x6a\xb1\xd0\xeb\x90\x65\x39\x96\x65\xb0\xb3\xf3" \
        "\x73\xce\xbc\x74\xe6\x47\xf1\x9b\x19\x20\x2b\x0a\xbe\xf8\xe2\x3a" \
        "\x95\x4a\x85\xb9\x46\x95\xfe\x60\x48\xb7\xdb\x65\x32\x1a\xa1\x94" \
        "\x62\x3a\x0d\x90\x52\x12\x85\x11\x7e\x90\x50\x14\x05\xfd\xbd\x3d" \
        "\xb2\x3c\xa3\xde\x70\x59\x5b\x5b\x65\xae\x39\xc7\xb3\xda\xdd\xb1" \
        "\x00\x52\xc0\x83\x47\x7b\x7c\xf8\xd1\xdf\x18\x8f\x27\x44\xbe\x07" \
        "\x42\xe1\xfb\x11\x85\x96\x74\xda\x6d\xb4\x50\xb8\xae\x4d\x51\x94" \
        "\x98\x56\x82\xeb\x3a\x0c\xfb\x03\x84\x90\x28\x25\x39\x7f\xfe\x3c" \
        "\x12\x28\x7e\x8a\x02\x02\xd8\xdd\xfd\x9a\xbd\xbd\x7d\x96\x16\x7b" \
        "\x84\xbe\x87\x46\x3e\xe9\x03\x9a\xfb\xf7\xf6\x88\x93\x84\x38\x76" \
        "\x28\x72\xa8\x56\x5d\xe6\xe7\x3b\x14\x79\x86\xd6\x70\xee\xdc\xf3" \
        "\x5c\xba\x74\x89\xe3\x7a\xfd\x89\x16\x5c\xbb\xf6\x25\xfd\x7e\x9f" \
        "\xb9\x46\x0d\x29\x25\xed\x76\x0b\x06\x23\x9a\xed\x16\x9e\x17\x81" \
        "\x00\x43\x29\x02\xcf\x47\x08\xf8\xd7\xed\x7f\x13\xc7\x09\x8d\xb9" \
        "\x3a\xa7\x37\x37\x58\x5e\x59\x39\x32\x7e\x27\x02\x08\x01\x5e\x10" \
        "\x70\xfd\xc6\x4d\x2a\x8e\x4d\x96\x66\x24\x49\x42\x9e\x17\x44\x51" \
        "\x8c\x32\x14\x68\x58\x5d\x5d\x24\x4f\x13\xa4\x94\xd4\xea\xd5\x43" \
        "\xf9\xa5\xa4\x2c\x33\x36\xd6\x9f\xc3\x32\x8c\x23\xe3\x77\x22\x80" \
        "\x7c\x22\xff\xd5\xcf\xae\xd2\xeb\x75\x48\xd3\x0c\xcb\xb2\x48\x93" \
        "\x84\x2c\xcb\x49\xa2\x98\x30\x88\x88\xa3\x94\xb2\x48\x31\x0c\x03" \
        "\x77\xbe\x45\xde\xa8\xe3\xe4\x05\xa7\xd6\x7a\x5c\xbe\x72\xe5\x24" \
        "\x81\x8f\xb7\xe0\xe6\xcd\x5b\x3c\x7c\xf0\x08\x29\x0c\x0a\xad\xe9" \
        "\x74\x5a\x20\x24\xcb\x2b\x8b\xa4\x69\x8a\x65\xd9\x28\x43\x31\x1a" \
        "\x44\x80\xe4\xe1\x83\x3d\x82\x20\xa2\x5a\xaf\xb2\xbd\xf5\x02\xdb" \
        "\x2f\xbe\xf8\xcc\xf8\x9d\x08\x90\xe5\x05\x37\x6f\xdc\xa2\xd1\x68" \
        "\x50\x6f\xcc\xb1\xbf\xdf\x27\x08\x62\xa2\x30\xc4\x77\x2c\xb4\x2e" \
        "\x69\xb7\x9b\x74\xe6\xbb\xa4\x69\x4a\xaf\xd7\x65\xd0\x1f\x20\xa4" \
        "\x40\x88\x92\x53\xa7\x56\xa9\xd7\x6a\xc7\xfa\xff\x4c\x00\x29\xe0" \
        "\xde\xc3\x87\x7c\xf0\xc1\x5f\x08\xc2\x14\x43\x05\x74\x3b\x73\x18" \
        "\x96\xc3\x74\x3c\x01\x34\xa3\xe1\x98\xd1\x70\x42\x7f\x30\xa1\x2c" \
        "\x72\xb2\x34\xc3\xa9\xb8\x28\x43\xd1\xee\x34\xb8\x70\xe1\x02\xe2" \
        "\x44\x03\x9e\x01\x70\x18\xbf\x5d\x06\xc3\x21\x8e\x63\x33\x19\x4f" \
        "\xb0\x2c\x93\x42\x2b\x1a\xcd\x26\xad\xa6\x4b\x51\xe4\xd4\x1b\x75" \
        "\x1e\xef\x1d\x00\x30\x1a\x4d\xf0\xbd\x90\x8a\x6b\xb1\xbe\x73\x96" \
        "\x57\x2f\x5e\x64\x96\x51\xeb\x48\x00\x0d\x7c\x7e\xed\x4b\x26\x93" \
        "\x29\xbd\x5e\x97\x40\x81\x53\xa9\xf0\xf8\xf1\x90\xf1\x68\xcc\xb0" \
        "\xbf\x8f\x69\x2a\x5c\xd7\xa5\x5a\xab\xd2\xed\xb6\xf1\xfd\x10\xdf" \
        "\x0f\x50\x0a\xd6\x7f\x76\x8a\x85\x85\xc5\x13\xe5\x87\x23\x46\xb2" \
        "\xc3\xdb\xcf\xe7\xda\xb5\x6b\x64\x69\x42\x7f\xbf\x4f\x92\xa4\xb8" \
        "\xd5\x0a\xcd\x66\x8d\x85\xc5\x0e\x08\x41\xa9\x05\x77\xff\x7b\x9f" \
        "\xc9\xd8\xc3\x0f\x42\x84\x28\x69\xb6\x1b\x74\xba\x2d\x5e\xd8\xda" \
        "\xc2\x54\xb3\x4d\x7b\xf2\xa8\x1f\xbb\xbb\xbb\xdc\xb8\x79\x8b\xc5" \
        "\xa5\x1e\xae\xeb\xa0\x81\x7b\x77\x1f\xe2\xf9\x01\xe8\x82\x7a\xdd" \
        "\xa5\x3b\xdf\xc2\xb4\x14\x52\x68\xa6\xe3\x31\x7b\x8f\x1e\x13\xf8" \
        "\x01\xed\x76\x8b\x2b\x33\xc4\xef\x58\x0b\xbe\xfa\xea\x16\x77\xee" \
        "\x3c\xc2\x75\x1d\x2c\xd3\xa0\xd9\x6e\xe2\xfb\x11\x59\x96\xb1\xbf" \
        "\xb7\x4f\x9e\x17\x28\x29\x31\x2d\x83\xd5\xce\x12\x42\x1c\xaa\xa1" \
        "\x94\xe2\xf4\xe6\x3a\xa7\x9f\x7f\xfe\xc4\xf8\x3d\x13\x20\xcd\x0b" \
        "\xae\xdf\xb8\x49\xad\xd1\xc0\x50\x8a\xe9\x64\x42\x56\x68\xa4\x54" \
        "\x74\x3b\x4d\x3c\xcf\x27\x89\x13\xc2\x20\x24\x0c\x02\x04\x1a\xcb" \
        "\xb6\x69\x34\x1b\x54\xaa\x15\x4e\x6f\x6e\x50\x75\xdd\x99\xfc\xff" \
        "\x11\x80\x14\x70\xe7\xfe\x3d\x3e\xfa\xf0\xcf\x58\xa6\xa2\x56\xab" \
        "\x61\x59\x26\x69\x9a\x32\x9d\x78\xa0\x0b\x40\xb3\xb4\xbc\x80\xe7" \
        "\x79\x24\x69\x4c\xe0\x07\x8c\xfa\x63\x84\x61\xd0\x68\xb8\x9c\x39" \
        "\x7b\x96\xa2\xd4\x08\x31\x4b\x08\x7f\x00\x20\x80\x7b\xf7\xef\x93" \
        "\xc4\x01\xa1\x37\xc1\x9f\x4e\x59\x5a\x5e\xc2\xb2\x4c\x2c\xcb\x24" \
        "\x8e\x22\xbc\xa9\x47\x59\x14\x08\x43\x31\xbf\xd0\xc5\x75\x1d\xee" \
        "\xfc\xe7\x2e\x52\xc1\xea\xca\x12\x4b\xcb\x2b\x0c\x87\x23\xea\xf5" \
        "\xfa\x93\x77\xc1\xf1\x4b\xfd\xfa\x37\xef\xbc\xf3\x34\x41\xbd\x56" \
        "\x67\x71\xb9\xc7\x78\x32\xe2\xe0\xf1\x3e\x81\x17\xe0\x79\x3e\xdd" \
        "\x6e\x07\xc3\x30\x10\x42\x20\xa4\xc4\x9b\x7a\x68\x01\xba\x28\x91" \
        "\x4a\xd2\x6c\xd6\x79\xe9\xa5\x2d\x76\x76\x76\xc8\x8b\x02\x21\x04" \
        "\xa6\x69\xa0\x94\x3a\x56\x8d\xef\x03\x00\xae\x5b\xe1\xdc\xd9\xb3" \
        "\xbc\xfc\xca\xcb\x38\xae\xcd\x64\x32\x66\xd0\x1f\x10\xf8\x3e\x41" \
        "\x10\xd2\x5b\xe8\x61\x3b\x0e\x49\x92\x52\x16\x05\xde\xd4\x23\x4d" \
        "\x12\x2c\x5b\xf1\xfa\x6b\x97\xd8\xd8\xd8\x20\x4b\x53\xf2\x3c\x07" \
        "\x0e\x21\xbe\x03\x9f\x09\xe0\x50\x09\xc1\x42\xaf\xc7\xc5\x8b\x17" \
        "\x59\xdf\x5c\x47\x8b\x92\xc9\x74\xc2\x68\x30\x22\x89\x53\xa2\x30" \
        "\xa6\x56\xab\xd2\x9d\xef\x12\xf8\x11\x5a\xc3\xf2\x52\x8f\x5f\xbe" \
        "\xf9\x26\x8d\x46\x9d\xa2\x28\xc8\xb2\x9c\x2c\xcf\x0f\x7d\x36\x0c" \
        "\x0c\xc3\x3c\x12\xe2\x68\x00\x0e\xbb\xa1\x65\x59\x9c\xde\xdc\xe4" \
        "\xc2\x2b\xaf\xd0\x6c\xcd\x31\x1a\x8f\x89\xe3\x98\xe9\x68\x8c\x2e" \
        "\x0b\xb4\x06\xa9\x14\xcd\x56\x93\xed\xad\x4d\xde\x78\xe3\x17\x48" \
        "\xa9\x00\x8d\x2e\x4b\x8a\x22\x27\xcf\x0f\x5f\x47\x87\x10\x06\x52" \
        "\x7e\x1f\xe2\xd8\xeb\xf8\xbb\x41\x72\x75\x75\x95\x5f\xbd\xfd\x36" \
        "\xaf\xee\xec\xf0\xa7\xf7\xdf\xe7\xdd\x3f\xfc\x91\x83\xfd\x7d\xa6" \
        "\xe3\x11\x85\x96\xd8\x8e\xc9\xd2\xe2\x02\x8e\xe3\xa0\xff\x3f\x7d" \
        "\x6a\x8a\x3c\x27\x0c\x43\x84\x10\x68\x0d\x5a\x6b\xaa\xd5\x2a\xea" \
        "\xa9\x2e\x39\xd3\x58\xae\x35\x98\x86\xc9\xab\x3b\x3b\xac\x9d\x3a" \
        "\xc5\xc6\xfa\x3a\xef\xbe\xf7\x1e\x7f\xff\xeb\xa7\xe4\x71\x46\x99" \
        "\xe7\xac\xaf\xaf\xa3\x94\xa2\x2c\xf5\x93\x2b\x59\x22\xe5\xe1\xa7" \
        "\x35\x94\x65\x49\x59\x16\x68\x5d\xf2\x74\x03\xfe\x1f\xc2\x60\x72" \
        "\xe2\x6a\x9b\x4e\x8f\x00\x00\x00\x00\x49\x45\x4e\x44\xae\x42\x60" \
        "\x82"
