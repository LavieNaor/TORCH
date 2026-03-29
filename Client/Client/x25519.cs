using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace TorClient
{
    public static class X25519
    {
        private static readonly BigInteger P = BigInteger.Pow(2, 255) - 19;
        private static readonly BigInteger A24 = 121665;

        // =========================
        // יצירת מפתח פרטי
        // =========================
        public static byte[] GeneratePrivateKey()
        {
            byte[] k = new byte[32];
            RandomNumberGenerator.Fill(k);

            // Clamping לפי RFC 7748
            k[0] &= 248;
            k[31] &= 127;
            k[31] |= 64;

            return k;
        }

        // =========================
        // יצירת מפתח ציבורי
        // =========================
        public static byte[] GeneratePublicKey(byte[] privateKey)
        {
            byte[] basePoint = new byte[32];
            basePoint[0] = 9;

            return ScalarMult(privateKey, basePoint);
        }

        // =========================
        // Shared Secret
        // =========================
        public static byte[] ComputeSharedSecret(byte[] privateKey, byte[] otherPublicKey)
        {
            return ScalarMult(privateKey, otherPublicKey);
        }

        // =========================
        // Scalar Multiplication
        // =========================
        private static byte[] ScalarMult(byte[] k, byte[] u)
        {
            BigInteger x1 = ToBigInt(u);
            BigInteger x2 = 1;
            BigInteger z2 = 0;
            BigInteger x3 = x1;
            BigInteger z3 = 1;

            int swap = 0;

            for (int t = 254; t >= 0; t--)
            {
                int kt = (k[t / 8] >> (t & 7)) & 1;
                swap ^= kt;
                ConditionalSwap(ref x2, ref x3, swap);
                ConditionalSwap(ref z2, ref z3, swap);
                swap = kt;

                BigInteger A = (x2 + z2) % P;
                BigInteger AA = (A * A) % P;
                BigInteger B = (x2 - z2 + P) % P;
                BigInteger BB = (B * B) % P;
                BigInteger E = (AA - BB + P) % P;
                BigInteger C = (x3 + z3) % P;
                BigInteger D = (x3 - z3 + P) % P;
                BigInteger DA = (D * A) % P;
                BigInteger CB = (C * B) % P;

                x3 = ((DA + CB) % P);
                x3 = (x3 * x3) % P;

                z3 = ((DA - CB + P) % P);
                z3 = (z3 * z3) % P;
                z3 = (z3 * x1) % P;

                x2 = (AA * BB) % P;
                z2 = (E * ((AA + (A24 * E) % P) % P)) % P;
            }

            ConditionalSwap(ref x2, ref x3, swap);
            ConditionalSwap(ref z2, ref z3, swap);

            BigInteger z2Inv = ModInverse(z2);
            BigInteger result = (x2 * z2Inv) % P;

            return ToBytes(result);
        }

        private static void ConditionalSwap(ref BigInteger a, ref BigInteger b, int swap)
        {
            if (swap == 1)
            {
                BigInteger temp = a;
                a = b;
                b = temp;
            }
        }

        private static BigInteger ModInverse(BigInteger x)
        {
            return BigInteger.ModPow(x, P - 2, P);
        }

        private static BigInteger ToBigInt(byte[] littleEndian)
        {
            return new BigInteger(littleEndian, isUnsigned: true, isBigEndian: false);
        }

        private static byte[] ToBytes(BigInteger value)
        {
            byte[] bytes = value.ToByteArray(isUnsigned: true, isBigEndian: false);

            Array.Resize(ref bytes, 32);
            return bytes;
        }
    }
}
