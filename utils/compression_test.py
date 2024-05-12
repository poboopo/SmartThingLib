import zlib
# Original string in byte format
original_string = b'{"cg": {"gtw": "192.168.2.114:8081","laddr": "192.168.2.114:7779"}}'
 
# Compressing the string using zlib
compressed_string = zlib.compress(original_string)
 
# Displaying the original and compressed string
print(f"Original String [{len(original_string)}]:", original_string)
print(f"Compressed String [{len(compressed_string)}]:", compressed_string)

