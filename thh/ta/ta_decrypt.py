import sys
from Crypto.Cipher import AES
from Crypto.Util.Padding import unpad

def decrypt_file(key_file, filename, iv_file):

    #Read key 
    with open(key_file, 'r') as file:
        key_hex = file.read().strip()
        key = bytes.fromhex(key_hex)

    # Read the file
    with open(filename, 'rb') as file:
	    # Read the initialization vector (IV)
	    #iv = file.read(16)
	    # Read the ciphertext
         ciphertext = file.read()

    # Read iv
    with open(iv_file, 'r') as file:
        iv_hex = file.read().strip()
        iv = bytes.fromhex(iv_hex)


    # Decrypt the ciphertext
    decryptor = AES.new(key, AES.MODE_CTR, nonce=iv)
    plaintext = decryptor.decrypt(ciphertext[:-473])
	
    # Write the decrypted plaintext to a new file
    with open(filename+".dec", 'wb') as decrypted_file:
	    decrypted_file.write(plaintext)


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python script_name.py  <key_file> <filename> <iv_file>")
        sys.exit(1)
    
    key_file = sys.argv[1]
    filename = sys.argv[2]
    iv_file = sys.argv[3]
    
    decrypt_file(key_file, filename, iv_file)