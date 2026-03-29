using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

public class Aes
{
    // consts
    private const int AES_KEY_SIZE = 16;
    private const int IV_SIZE = 16;
    private const int SALT_SIZE = 16;


    // basic SBox
    private static readonly byte[] SBox = new byte[]
    {
    0x63,0x7C,0x77,0x7B,0xF2,0x6B,0x6F,0xC5,0x30,0x01,0x67,0x2B,0xFE,0xD7,0xAB,0x76,
    0xCA,0x82,0xC9,0x7D,0xFA,0x59,0x47,0xF0,0xAD,0xD4,0xA2,0xAF,0x9C,0xA4,0x72,0xC0,
    0xB7,0xFD,0x93,0x26,0x36,0x3F,0xF7,0xCC,0x34,0xA5,0xE5,0xF1,0x71,0xD8,0x31,0x15,
    0x04,0xC7,0x23,0xC3,0x18,0x96,0x05,0x9A,0x07,0x12,0x80,0xE2,0xEB,0x27,0xB2,0x75,
    0x09,0x83,0x2C,0x1A,0x1B,0x6E,0x5A,0xA0,0x52,0x3B,0xD6,0xB3,0x29,0xE3,0x2F,0x84,
    0x53,0xD1,0x00,0xED,0x20,0xFC,0xB1,0x5B,0x6A,0xCB,0xBE,0x39,0x4A,0x4C,0x58,0xCF,
    0xD0,0xEF,0xAA,0xFB,0x43,0x4D,0x33,0x85,0x45,0xF9,0x02,0x7F,0x50,0x3C,0x9F,0xA8,
    0x51,0xA3,0x40,0x8F,0x92,0x9D,0x38,0xF5,0xBC,0xB6,0xDA,0x21,0x10,0xFF,0xF3,0xD2,
    0xCD,0x0C,0x13,0xEC,0x5F,0x97,0x44,0x17,0xC4,0xA7,0x7E,0x3D,0x64,0x5D,0x19,0x73,
    0x60,0x81,0x4F,0xDC,0x22,0x2A,0x90,0x88,0x46,0xEE,0xB8,0x14,0xDE,0x5E,0x0B,0xDB,
    0xE0,0x32,0x3A,0x0A,0x49,0x06,0x24,0x5C,0xC2,0xD3,0xAC,0x62,0x91,0x95,0xE4,0x79,
    0xE7,0xC8,0x37,0x6D,0x8D,0xD5,0x4E,0xA9,0x6C,0x56,0xF4,0xEA,0x65,0x7A,0xAE,0x08,
    0xBA,0x78,0x25,0x2E,0x1C,0xA6,0xB4,0xC6,0xE8,0xDD,0x74,0x1F,0x4B,0xBD,0x8B,0x8A,
    0x70,0x3E,0xB5,0x66,0x48,0x03,0xF6,0x0E,0x61,0x35,0x57,0xB9,0x86,0xC1,0x1D,0x9E,
    0xE1,0xF8,0x98,0x11,0x69,0xD9,0x8E,0x94,0x9B,0x1E,0x87,0xE9,0xCE,0x55,0x28,0xDF,
    0x8C,0xA1,0x89,0x0D,0xBF,0xE6,0x42,0x68,0x41,0x99,0x2D,0x0F,0xB0,0x54,0xBB,0x16
    };

    // reverse SBox
    private static readonly byte[] InvSBox = new byte[]
    {
    0x52,0x09,0x6A,0xD5,0x30,0x36,0xA5,0x38,0xBF,0x40,0xA3,0x9E,0x81,0xF3,0xD7,0xFB,
    0x7C,0xE3,0x39,0x82,0x9B,0x2F,0xFF,0x87,0x34,0x8E,0x43,0x44,0xC4,0xDE,0xE9,0xCB,
    0x54,0x7B,0x94,0x32,0xA6,0xC2,0x23,0x3D,0xEE,0x4C,0x95,0x0B,0x42,0xFA,0xC3,0x4E,
    0x08,0x2E,0xA1,0x66,0x28,0xD9,0x24,0xB2,0x76,0x5B,0xA2,0x49,0x6D,0x8B,0xD1,0x25,
    0x72,0xF8,0xF6,0x64,0x86,0x68,0x98,0x16,0xD4,0xA4,0x5C,0xCC,0x5D,0x65,0xB6,0x92,
    0x6C,0x70,0x48,0x50,0xFD,0xED,0xB9,0xDA,0x5E,0x15,0x46,0x57,0xA7,0x8D,0x9D,0x84,
    0x90,0xD8,0xAB,0x00,0x8C,0xBC,0xD3,0x0A,0xF7,0xE4,0x58,0x05,0xB8,0xB3,0x45,0x06,
    0xD0,0x2C,0x1E,0x8F,0xCA,0x3F,0x0F,0x02,0xC1,0xAF,0xBD,0x03,0x01,0x13,0x8A,0x6B,
    0x3A,0x91,0x11,0x41,0x4F,0x67,0xDC,0xEA,0x97,0xF2,0xCF,0xCE,0xF0,0xB4,0xE6,0x73,
    0x96,0xAC,0x74,0x22,0xE7,0xAD,0x35,0x85,0xE2,0xF9,0x37,0xE8,0x1C,0x75,0xDF,0x6E,
    0x47,0xF1,0x1A,0x71,0x1D,0x29,0xC5,0x89,0x6F,0xB7,0x62,0x0E,0xAA,0x18,0xBE,0x1B,
    0xFC,0x56,0x3E,0x4B,0xC6,0xD2,0x79,0x20,0x9A,0xDB,0xC0,0xFE,0x78,0xCD,0x5A,0xF4,
    0x1F,0xDD,0xA8,0x33,0x88,0x07,0xC7,0x31,0xB1,0x12,0x10,0x59,0x27,0x80,0xEC,0x5F,
    0x60,0x51,0x7F,0xA9,0x19,0xB5,0x4A,0x0D,0x2D,0xE5,0x7A,0x9F,0x93,0xC9,0x9C,0xEF,
    0xA0,0xE0,0x3B,0x4D,0xAE,0x2A,0xF5,0xB0,0xC8,0xEB,0xBB,0x3C,0x83,0x53,0x99,0x61,
    0x17,0x2B,0x04,0x7E,0xBA,0x77,0xD6,0x26,0xE1,0x69,0x14,0x63,0x55,0x21,0x0C,0x7D
    };

    private static readonly byte[] Rcon = new byte[]
    {
    0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36
    };

    /// <summary>
    /// This function encrypt the data
    /// </summary>
    /// <param name="key"> The AES key </param>
    /// <param name="plaintext"> The text to encrypt </param>
    /// <param name="workload"> How many rounds </param>
    /// <returns> Decrypt text </returns>
    public static byte[] Encrypt(string key, string plaintext, int workload = 100000)
    {
        // turn the key to bytes format
        byte[] keyBytes = Encoding.UTF8.GetBytes(key);
        byte[] plaintextBytes = Encoding.UTF8.GetBytes(plaintext);

        // add salt to the 
        byte[] salt = new byte[SALT_SIZE];
        using (var rng = RandomNumberGenerator.Create())
        {
            rng.GetBytes(salt);
        }

        // expend key
        byte[] derived = PBKDF2(keyBytes, salt, workload, AES_KEY_SIZE + IV_SIZE);
        byte[] aesKey = new byte[AES_KEY_SIZE];
        byte[] iv = new byte[IV_SIZE];
        Array.Copy(derived, 0, aesKey, 0, AES_KEY_SIZE);
        Array.Copy(derived, AES_KEY_SIZE, iv, 0, IV_SIZE);

        byte[][] roundKeys = KeyExpansion(aesKey);

        // enctypt the text
        byte[] ciphertext = EncryptCBC(plaintextBytes, iv, roundKeys);

        byte[] result = new byte[SALT_SIZE + ciphertext.Length];
        Array.Copy(salt, 0, result, 0, SALT_SIZE);
        Array.Copy(ciphertext, 0, result, SALT_SIZE, ciphertext.Length);

        return result;
    }

    /// <summary>
    /// The function dectypt the text
    /// </summary>
    /// <param name="key"> The AES key</param>
    /// <param name="ciphertext"> The text to Decrypt </param>
    /// <param name="workload"> How many rounds </param>
    /// <returns> The plain text </returns>
    /// <exception cref="ArgumentException"></exception>
    public static byte[] Decrypt(string key, byte[] ciphertext, int workload = 100000)
    {
        // check if it is posibble to decrypt
        if (ciphertext.Length % 16 != 0)
            throw new ArgumentException("Ciphertext must be made of full 16-byte blocks.");
        if (ciphertext.Length < 32)
            throw new ArgumentException("Ciphertext must be at least 32 bytes long.");

        // turn the aes key to bytes format
        byte[] keyBytes = Encoding.UTF8.GetBytes(key);


        // expend the key
        byte[] salt = new byte[SALT_SIZE];
        Array.Copy(ciphertext, 0, salt, 0, SALT_SIZE);

        byte[] encData = new byte[ciphertext.Length - SALT_SIZE];
        Array.Copy(ciphertext, SALT_SIZE, encData, 0, encData.Length);

        byte[] derived = PBKDF2(keyBytes, salt, workload, AES_KEY_SIZE + IV_SIZE);
        byte[] aesKey = new byte[AES_KEY_SIZE];
        byte[] iv = new byte[IV_SIZE];
        Array.Copy(derived, 0, aesKey, 0, AES_KEY_SIZE);
        Array.Copy(derived, AES_KEY_SIZE, iv, 0, IV_SIZE);

        byte[][] roundKeys = KeyExpansion(aesKey);

        // dectypt
        return DecryptCBC(encData, iv, roundKeys);
    }

    private static byte[] PBKDF2(byte[] password, byte[] salt, int iterations, int dkLen)
    {
        using (var hmac = new HMACSHA256(password))
        {
            int hashLen = 32;
            int blocks = (dkLen + hashLen - 1) / hashLen;
            byte[] result = new byte[blocks * hashLen];

            for (int blockIndex = 1; blockIndex <= blocks; blockIndex++)
            {
                byte[] blockBytes = new byte[salt.Length + 4];
                Array.Copy(salt, 0, blockBytes, 0, salt.Length);
                blockBytes[salt.Length + 0] = (byte)((blockIndex >> 24) & 0xFF);
                blockBytes[salt.Length + 1] = (byte)((blockIndex >> 16) & 0xFF);
                blockBytes[salt.Length + 2] = (byte)((blockIndex >> 8) & 0xFF);
                blockBytes[salt.Length + 3] = (byte)(blockIndex & 0xFF);

                byte[] u = hmac.ComputeHash(blockBytes);
                byte[] xorResult = (byte[])u.Clone();

                for (int i = 1; i < iterations; i++)
                {
                    u = hmac.ComputeHash(u);
                    for (int j = 0; j < hashLen; j++)
                        xorResult[j] ^= u[j];
                }

                Array.Copy(xorResult, 0, result, (blockIndex - 1) * hashLen, hashLen);
            }

            byte[] output = new byte[dkLen];
            Array.Copy(result, 0, output, 0, dkLen);
            return output;
        }
    }

    /// <summary>
    /// Encrypts `plaintext` using CBC mode and PKCS#7 padding, with the given initialization vector(iv).
    /// </summary>
    /// <param name="plaintext"></param>
    /// <param name="iv"></param>
    /// <param name="roundKeys"></param>
    /// <returns></returns>
    private static byte[] EncryptCBC(byte[] plaintext, byte[] iv, byte[][] roundKeys)
    {
        byte[] padded = PadPKCS7(plaintext);
        byte[] result = new byte[padded.Length];
        byte[] previous = (byte[])iv.Clone();

        for (int i = 0; i < padded.Length; i += 16)
        {
            byte[] block = new byte[16];
            Array.Copy(padded, i, block, 0, 16);

            for (int j = 0; j < 16; j++)
                block[j] ^= previous[j];

            byte[] encrypted = EncryptBlock(block, roundKeys);
            Array.Copy(encrypted, 0, result, i, 16);
            previous = encrypted;
        }

        return result;
    }

    /// <summary>
    /// Decrypts `ciphertext` using CBC mode and PKCS#7 padding, with the given initialization vector(iv)
    /// </summary>
    /// <param name="ciphertext"></param>
    /// <param name="iv"></param>
    /// <param name="roundKeys"></param>
    /// <returns></returns>
    private static byte[] DecryptCBC(byte[] ciphertext, byte[] iv, byte[][] roundKeys)
    {
        byte[] result = new byte[ciphertext.Length];
        byte[] previous = (byte[])iv.Clone();

        for (int i = 0; i < ciphertext.Length; i += 16)
        {
            byte[] block = new byte[16];
            Array.Copy(ciphertext, i, block, 0, 16);

            byte[] decrypted = DecryptBlock(block, roundKeys);

            for (int j = 0; j < 16; j++)
                decrypted[j] ^= previous[j];

            Array.Copy(decrypted, 0, result, i, 16);
            previous = block;
        }

        return UnpadPKCS7(result);
    }

    // This function expend the key
    private static byte[][] KeyExpansion(byte[] key)
    {
        int rounds = 10;
        int words = 4;
        byte[][] w = new byte[4 * (rounds + 1)][];

        for (int i = 0; i < words; i++)
        {
            w[i] = new byte[] { key[4 * i], key[4 * i + 1], key[4 * i + 2], key[4 * i + 3] };
        }

        for (int i = words; i < 4 * (rounds + 1); i++)
        {
            byte[] temp = (byte[])w[i - 1].Clone();

            if (i % words == 0)
            {
                temp = RotWord(temp);
                temp = SubWord(temp);
                temp[0] ^= Rcon[i / words];
            }

            w[i] = new byte[4];
            for (int j = 0; j < 4; j++)
                w[i][j] = (byte)(w[i - words][j] ^ temp[j]);
        }

        byte[][] roundKeys = new byte[rounds + 1][];
        for (int r = 0; r <= rounds; r++)
        {
            roundKeys[r] = new byte[16];
            for (int c = 0; c < 4; c++)
                for (int row = 0; row < 4; row++)
                    roundKeys[r][row + 4 * c] = w[4 * r + c][row];
        }

        return roundKeys;
    }

    private static byte[] RotWord(byte[] word)
    {
        return new byte[] { word[1], word[2], word[3], word[0] };
    }

    private static byte[] SubWord(byte[] word)
    {
        for (int i = 0; i < 4; i++)
            word[i] = SBox[word[i]];
        return word;
    }

    // Encrypts a single block of 16 byte long plaintext.
    private static byte[] EncryptBlock(byte[] input, byte[][] roundKeys)
    {
        byte[,] state = new byte[4, 4];
        for (int c = 0; c < 4; c++)
            for (int r = 0; r < 4; r++)
                state[r, c] = input[r + 4 * c];

        // expend the key
        AddRoundKey(state, roundKeys[0]);

        // enctypt in 9 rounds
        for (int round = 1; round <= 9; round++)
        {
            SubBytes(state);
            ShiftRows(state);
            MixColumns(state);
            AddRoundKey(state, roundKeys[round]);
        }

        // encrypt the 10th round
        SubBytes(state);
        ShiftRows(state);
        AddRoundKey(state, roundKeys[10]);

        byte[] output = new byte[16];
        for (int c = 0; c < 4; c++)
            for (int r = 0; r < 4; r++)
                output[r + 4 * c] = state[r, c];

        return output;
    }

    // Decrypts a single block of 16 byte long ciphertext.
    private static byte[] DecryptBlock(byte[] input, byte[][] roundKeys)
    {
        byte[,] state = new byte[4, 4];
        for (int c = 0; c < 4; c++)
            for (int r = 0; r < 4; r++)
                state[r, c] = input[r + 4 * c];

        // decrypt the 10th round
        AddRoundKey(state, roundKeys[10]);
        InvShiftRows(state);
        InvSubBytes(state);


        // decrypt in 9 rounds
        for (int round = 9; round >= 1; round--)
        {
            AddRoundKey(state, roundKeys[round]);
            InvMixColumns(state);
            InvShiftRows(state);
            InvSubBytes(state);
        }

        AddRoundKey(state, roundKeys[0]);

        byte[] output = new byte[16];
        for (int c = 0; c < 4; c++)
            for (int r = 0; r < 4; r++)
                output[r + 4 * c] = state[r, c];

        return output;
    }

    // sub the bytes according to SBox
    private static void SubBytes(byte[,] state)
    {
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                state[r, c] = SBox[state[r, c]];
    }

    // sub the bytes according to InvSBox
    private static void InvSubBytes(byte[,] state)
    {
        for (int r = 0; r < 4; r++)
            for (int c = 0; c < 4; c++)
                state[r, c] = InvSBox[state[r, c]];
    }

    // replace rows according to legality of AES encrypt
    private static void ShiftRows(byte[,] state)
    {
        byte temp;

        temp = state[1, 0];
        state[1, 0] = state[1, 1];
        state[1, 1] = state[1, 2];
        state[1, 2] = state[1, 3];
        state[1, 3] = temp;

        temp = state[2, 0];
        state[2, 0] = state[2, 2];
        state[2, 2] = temp;
        temp = state[2, 1];
        state[2, 1] = state[2, 3];
        state[2, 3] = temp;

        temp = state[3, 3];
        state[3, 3] = state[3, 2];
        state[3, 2] = state[3, 1];
        state[3, 1] = state[3, 0];
        state[3, 0] = temp;
    }

    // replace rows according to legality of AES decrypt
    private static void InvShiftRows(byte[,] state)
    {
        byte temp;

        temp = state[1, 3];
        state[1, 3] = state[1, 2];
        state[1, 2] = state[1, 1];
        state[1, 1] = state[1, 0];
        state[1, 0] = temp;

        temp = state[2, 0];
        state[2, 0] = state[2, 2];
        state[2, 2] = temp;
        temp = state[2, 1];
        state[2, 1] = state[2, 3];
        state[2, 3] = temp;

        temp = state[3, 0];
        state[3, 0] = state[3, 1];
        state[3, 1] = state[3, 2];
        state[3, 2] = state[3, 3];
        state[3, 3] = temp;
    }

    // mix columns according to legality of AES encrypt
    private static void MixColumns(byte[,] state)
    {
        for (int c = 0; c < 4; c++)
        {
            byte s0 = state[0, c], s1 = state[1, c], s2 = state[2, c], s3 = state[3, c];

            state[0, c] = (byte)(GFMul(0x02, s0) ^ GFMul(0x03, s1) ^ s2 ^ s3);
            state[1, c] = (byte)(s0 ^ GFMul(0x02, s1) ^ GFMul(0x03, s2) ^ s3);
            state[2, c] = (byte)(s0 ^ s1 ^ GFMul(0x02, s2) ^ GFMul(0x03, s3));
            state[3, c] = (byte)(GFMul(0x03, s0) ^ s1 ^ s2 ^ GFMul(0x02, s3));
        }
    }

    // mix columns according to legality of AES decrypt
    private static void InvMixColumns(byte[,] state)
    {
        for (int c = 0; c < 4; c++)
        {
            byte s0 = state[0, c], s1 = state[1, c], s2 = state[2, c], s3 = state[3, c];

            state[0, c] = (byte)(GFMul(0x0E, s0) ^ GFMul(0x0B, s1) ^ GFMul(0x0D, s2) ^ GFMul(0x09, s3));
            state[1, c] = (byte)(GFMul(0x09, s0) ^ GFMul(0x0E, s1) ^ GFMul(0x0B, s2) ^ GFMul(0x0D, s3));
            state[2, c] = (byte)(GFMul(0x0D, s0) ^ GFMul(0x09, s1) ^ GFMul(0x0E, s2) ^ GFMul(0x0B, s3));
            state[3, c] = (byte)(GFMul(0x0B, s0) ^ GFMul(0x0D, s1) ^ GFMul(0x09, s2) ^ GFMul(0x0E, s3));
        }
    }

    // This function is used for the expend key
    private static void AddRoundKey(byte[,] state, byte[] roundKey)
    {
        for (int c = 0; c < 4; c++)
            for (int r = 0; r < 4; r++)
                state[r, c] ^= roundKey[r + 4 * c];
    }

    private static byte GFMul(byte a, byte b)
    {
        byte result = 0;
        byte hi;

        for (int i = 0; i < 8; i++)
        {
            if ((b & 1) != 0)
                result ^= a;

            hi = (byte)(a & 0x80);
            a <<= 1;
            if (hi != 0)
                a ^= 0x1B;
            b >>= 1;
        }

        return result;
    }

    /*
    Pads the given plaintext with PKCS#7 padding to a multiple of 16 bytes.
    Note that if the plaintext size is a multiple of 16,
    a whole block will be added.
     */
    private static byte[] PadPKCS7(byte[] data)
    {
        int padLen = 16 - (data.Length % 16);
        byte[] padded = new byte[data.Length + padLen];
        Array.Copy(data, padded, data.Length);
        for (int i = data.Length; i < padded.Length; i++)
            padded[i] = (byte)padLen;
        return padded;
    }

    /*
     Removes a PKCS#7 padding, returning the unpadded text and ensuring the
     padding was correct.
     */
    private static byte[] UnpadPKCS7(byte[] data)
    {
        int padLen = data[data.Length - 1];
        if (padLen < 1 || padLen > 16)
            throw new Exception("Invalid PKCS7 padding.");

        for (int i = data.Length - padLen; i < data.Length; i++)
            if (data[i] != padLen)
                throw new Exception("Invalid PKCS7 padding.");

        byte[] unpadded = new byte[data.Length - padLen];
        Array.Copy(data, unpadded, unpadded.Length);
        return unpadded;
    }

    // The function encode the text in base64
    public static string EncryptToBase64(string text, string key)
    {
        string b64Text = Convert.ToBase64String(Encoding.UTF8.GetBytes(text));
        byte[] ciphertext = Encrypt(key, b64Text);
        return Convert.ToBase64String(ciphertext);
    }

    // The function decode the text in base64
    public static string DecryptFromBase64(string text, string key)
    {
        byte[] b = Convert.FromBase64String(text);
        byte[] decryptedMsg = Aes.Decrypt(key, b);

        byte[] finalBytes = Convert.FromBase64String(
            Encoding.UTF8.GetString(decryptedMsg)
        );

        string result = Encoding.UTF8.GetString(finalBytes);

        return result;
    }

    // The function turn the bytes to HEX format
    public static string ToHex(byte[] bytes)
    {
        StringBuilder sb = new StringBuilder(bytes.Length * 2);
        foreach (byte b in bytes)
            sb.AppendFormat("{0:X2}", b);
        return sb.ToString();
    }

    // The function turn the HEX format to bytes
    public static byte[] FromHex(string hex)
    {
        byte[] bytes = new byte[hex.Length / 2];
        for (int i = 0; i < bytes.Length; i++)
            bytes[i] = Convert.ToByte(hex.Substring(i * 2, 2), 16);
        return bytes;
    }
}

