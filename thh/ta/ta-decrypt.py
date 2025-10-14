import sys
from Crypto.Cipher import AES
from Crypto.Util.Padding import unpad

def decrypt_file(key, filename, iv):

	# Read the file
	with open(filename, 'rb') as file:
		# Read the initialization vector (IV)
		#iv = file.read(16)
		# Read the ciphertext
		ciphertext = file.read()

	
	# Decrypt the ciphertext
	decryptor = AES.new(key, AES.MODE_CTR, nonce=iv)
	plaintext = decryptor.decrypt(ciphertext[:-473])
	
	# Write the decrypted plaintext to a new file
	with open(filename+".dec", 'wb') as decrypted_file:
		decrypted_file.write(plaintext)

if __name__ == "__main__":
	if len(sys.argv) != 2:
		print("Usage: python script_name.py <filename>")
		sys.exit(1)
	key = bytes.fromhex("22b5e1352160fa8de6a5c8837855a4b8")
	iv=bytes.fromhex("ff 74 5f c8 ea 5e 65 12")
	filename = sys.argv[1]
	decrypt_file(key, filename, iv)

