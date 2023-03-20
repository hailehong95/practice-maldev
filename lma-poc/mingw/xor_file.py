# Use: ./xor.py file1 xor_key outfile
# Example: ./xor.py C:\a.exe 6 C:\result.exe

import sys

# Read files as byte arrays
file_bin = bytearray(open(sys.argv[1], 'rb').read())
xor_key = int(sys.argv[2])
file_enc = sys.argv[3]

# Set the length to be the smaller one
size = len(file_bin)
payload_encoded = bytearray(size)

# XOR Encode
for i in range(size):
	payload_encoded[i] = file_bin[i] ^ xor_key

# Write the XORd bytes to the output file
open(file_enc, 'wb').write(payload_encoded)

print("[*] {0} XOR {1}\n[*] Saved to {2}".format(sys.argv[1], xor_key, file_enc))
