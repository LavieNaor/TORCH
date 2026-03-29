import aes
import base64


def main():

    key1 = "5468617473206d79204b756e67204675"
    key2 = "5468617473206d79204b756e67204674"
    key3 = "5468617473206d79204b756e67204673"

    # ----------------------- ENCRYPTION ----------------------- #

    # transform the plaintext to base64 before the first use!
    # plain text

    plaintext = "Hello my name is Lavie! 12345"
    # plaintexthex = base64.b64encode(plaintext).decode()
    print(base64.b64encode(plaintext.encode()).decode(), "\n")

    # encryption with key 1
    ciphertext1 = aes.encrypt(key1, base64.b64encode(plaintext.encode()).decode())
    ciphertext1hex = base64.b64encode(ciphertext1).decode()
    print(ciphertext1hex, "\n")


    # encryption with key 2
    ciphertext2 = aes.encrypt(key2, base64.b64encode(ciphertext1hex.encode()).decode())
    ciphertext2hex = base64.b64encode(ciphertext2).decode()
    print(ciphertext2hex, "\n")

    # encryption with key 3
    ciphertext3 = aes.encrypt(key3, base64.b64encode(ciphertext2hex.encode()).decode())
    ciphertext3hex = base64.b64encode(ciphertext3).decode()
    print(ciphertext3hex, "\n")

    # ----------------------- DECRYPTION ----------------------- #

    # decryption with key 3
    plaintext3 = aes.decrypt(key3, base64.b64decode(ciphertext3hex))
    plaintext3hex = base64.b64decode(plaintext3).decode()
    print(plaintext3hex, "\n")

    # decryption with key 2
    plaintext2 = aes.decrypt(key2, base64.b64decode(plaintext3hex))
    plaintext2hex = base64.b64decode(plaintext2).decode()
    print(plaintext2hex, "\n")

    # decryption with key 1
    plaintext1 = aes.decrypt(key1, base64.b64decode(plaintext2hex))
    plaintext1hex = base64.b64decode(plaintext1).decode()
    print(plaintext1hex, "\n")



if __name__ == '__main__':
    main()
