using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Buffers.Text;
using System.Collections;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Net.Security;
using System.Net.Sockets;
using System.Reflection;
using System.Security.Authentication;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;
using static System.Net.Mime.MediaTypeNames;

namespace TorClient
{
    internal class Helper
    {

        private const String SERVER_IP = "tor_server"; // "127.0.0.1";
        private const String NODE1_IP = "tor_node1"; // "127.0.0.1";
        private const String NODE2_IP = "tor_node2"; // "127.0.0.1";
        private const String NODE3_IP = "tor_node3"; // "127.0.0.1";

        private const int NODE1_PORT = 7000;
        private const int NODE2_PORT = 7100;
        private const int NODE3_PORT = 7200;
        private const int SERVER_PORT = 8000;

        private static ConcurrentQueue<JObject> _queue = new ConcurrentQueue<JObject>();

        private static Random rand = new Random();

        private static bool _isFirstMessage = true;

        /*
         * The function random the nodes
         * input: none
         * output: array of nodes randomed
         */ 
        static public Node[] randNodes()
        {
            Node node1 = new Node(NODE1_IP, NODE1_PORT);
            Node node2 = new Node(NODE2_IP, NODE2_PORT);
            Node node3 = new Node(NODE3_IP, NODE3_PORT);

            Node[] nodes = { node1, node2, node3 };

            int[] numbers = { 0, 1, 2 };

            for (int i = numbers.Length - 1; i > 0; i--)
            {
                int j = rand.Next(i + 1);

                int temp = numbers[i];
                numbers[i] = numbers[j];
                numbers[j] = temp;
            }

            Node[] randNodes = { nodes[numbers[0]], nodes[numbers[1]], nodes[numbers[2]] };

            return randNodes;

        }

        /*
         * The function create conection with the entry node
         * input: clent
         * output: the ssl socket (stream) between the client and the entry node
         */
        static public SslStream createConnection(Client client)
        {
            TcpClient sock = new TcpClient("127.0.0.1", client.entryNode.port);

            SslStream sslStream = new SslStream(
                sock.GetStream(),
                false,
                new RemoteCertificateValidationCallback(ValidateServerCertificate),
                null
            );

            sslStream.AuthenticateAsClient("127.0.0.1");

            return sslStream;
        }

        /*
         * The function build the public key layer (client_public = base64 -> publicKey)
         * input: public key
         * output: string of the json public layer
         */
        static private String buildPublicKeyLayer(byte[] publicKey)
        {
            string base64 = Convert.ToBase64String(publicKey);

            string json = JsonConvert.SerializeObject(new
            {
                client_public = base64
            });

            return json;
        }

        /*
         * Function that send message through socket
         * input: client stream (socket), message to send
         * output: none
         */
        static public void sendMsg(SslStream clientStream, String msg)
        {
            byte[] buffer = new ASCIIEncoding().GetBytes(msg.Length.ToString("D10") + msg);
            clientStream.Write(buffer, 0, buffer.Length);
            clientStream.Flush();
        }

        static private int getSizeMessage(SslStream clientStream)
        {
            byte[] buffer;
            int bytesRead = 0;
            string answer = "";
            string lengthAnswer = "";

            buffer = new byte[10];
            bytesRead = clientStream.Read(buffer, 0, 10);
            answer = Encoding.ASCII.GetString(buffer, 0, bytesRead);

            lengthAnswer += answer[0];
            lengthAnswer += answer[1];
            lengthAnswer += answer[2];
            lengthAnswer += answer[3];
            lengthAnswer += answer[4];
            lengthAnswer += answer[5];
            lengthAnswer += answer[6];
            lengthAnswer += answer[7];
            lengthAnswer += answer[8];
            lengthAnswer += answer[9];

            return int.Parse(lengthAnswer);
        }

        /*
         * Function that receive message through socket
         * input: Client stream (socket)
         * output: The message that was received
         */
        static public String receiveMsg(SslStream clientStream)
        {
            String answer = "";
            byte[] buffer;
            int bytesRead = 0, sizeRead = 0;

            sizeRead = getSizeMessage(clientStream);

            buffer = new byte[sizeRead];
            bytesRead = clientStream.Read(buffer, 0, sizeRead);

            answer = Encoding.UTF8.GetString(buffer, 0, bytesRead);

            return answer;
        }

        /*
         * Function that do handshake with node and return the aes key
         * input: client, num of layers (Node 1 - 0, Node 2 - 1, Node 3 - 2)
         * output: The aes key
         */
        static private byte[] handshakeWithOneNode(Client client, int numOfLayers)
        {
            byte[] privateKey = { };
            byte[] publicKey = { };
            String publicKeyLayer = "", layers = "";
            String answer = "";
            JObject j;

            privateKey = X25519.GeneratePrivateKey();
            publicKey = X25519.GeneratePublicKey(privateKey);

            publicKeyLayer = buildPublicKeyLayer(publicKey);

            layers = createNumOfLayers(publicKeyLayer, client, numOfLayers);

            sendMsg(client.entryNodeSock, layers);

            answer = receiveMsg(client.entryNodeSock);

            j = release_num_of_layers(answer, client, numOfLayers);

            return getAesKey(client, j, privateKey);
        }

        /*
         * Function that create aes key by public of the node and the private key of the client
         * input: client, json of the response, private key
         * output: The aes key
         */
        static private byte[] getAesKey(Client client, JObject j, byte[] privateKey)
        {
            byte[] aesKey = { };
            byte[] sharedSecret = { };
            byte[] node_public = { };

            node_public = Convert.FromBase64String(j["node_public"].ToString());

            sharedSecret = X25519.ComputeSharedSecret(privateKey, node_public);

            aesKey = HKDF.DeriveKey(
                HashAlgorithmName.SHA256,
                sharedSecret,
                16,
                null,
                Encoding.UTF8.GetBytes("tor-session")
            );

            System.Diagnostics.Debug.WriteLine("Shared: " + Convert.ToBase64String(sharedSecret));
            System.Diagnostics.Debug.WriteLine("AES key: " + Convert.ToBase64String(aesKey));

            return aesKey;
        }

        /*
         * Function that build layers of onion by the client on the message
         * input: message to build onion on, client
         * output: encrypted message by the layers
         */
        static private String buildOnionWithoutPorts(String message, Client client)
        {
            String enc3 = build_layer_without_port(message, client.exitNode.key);
            String enc2 = build_layer_without_port(enc3, client.middleNode.key);
            String enc1 = build_layer_without_port(enc2, client.entryNode.key);

            return enc1;
        }

        /*
         * Function that build layers of onion by the client on the message
         * input: message to build onion on, client
         * output: encrypted message by the layers
         */
        static private String buildOnionWithPorts(String message, Client client)
        {
            String enc3 = build_layer_with_port(message, client.exitNode.key, SERVER_IP, SERVER_PORT);
            String enc2 = build_layer_with_port(enc3, client.middleNode.key, client.exitNode.ip, client.exitNode.port);
            String enc1 = build_layer_with_port(enc2, client.entryNode.key, client.middleNode.ip, client.middleNode.port);

            return enc1;
        }

        /*
         * Function that build layer by key on the message
         * input: message to build layer on, key to encrypt with, ip, port
         * output: encrypted message by the layer
         */
        static private String build_layer_with_port(String message, byte[] key, String ip, int port)
        {
            String jsonString = JsonConvert.SerializeObject(new
            {
                ip = ip,
                port = port,
                message = message
            });

            String enc = Aes.EncryptToBase64(jsonString, Convert.ToBase64String(key));

            return enc;
        }

        /*
         * Function that build layer by key on the message
         * input: message to build layer on, key to encrypt with
         * output: encrypted message by the layer
         */
        static private String build_layer_without_port(String message, byte[] key)
        {
            String jsonString = JsonConvert.SerializeObject(new
            {
                message = message
            });

            String enc = Aes.EncryptToBase64(jsonString, Convert.ToBase64String(key));

            return enc;
        }

        /*
         * Function that build layers of onion by the client and by the num of layer on the client public
         * input: client public to build onion on, client, num of layers to build
         * output: encrypted client public by the layers
         */
        static private String createNumOfLayers(String first_layer, Client client, int num)
        {
            String enc = first_layer;
            if(num == 0)
            {
                return enc;
            }
            if(num >= 3)
            {
                enc = build_layer_with_port(enc, client.exitNode.key, SERVER_IP, SERVER_PORT);
            }
            if(num >= 2)
            {
                enc = build_layer_with_port(enc, client.middleNode.key, client.exitNode.ip, client.exitNode.port);
            }
            if(num >= 1)
            {
                enc = build_layer_with_port(enc, client.entryNode.key, client.middleNode.ip, client.middleNode.port);
            }

            return enc;
        }

        /*
         * Function that release layers of onion by the client and by the num of layer from data
         * input: client public to release layers from, client, num of layer to release
         * output: JObject of the data after decrypt the layers
         */
        static private JObject release_num_of_layers(String data, Client client, int num)
        {
            String msg = data;

            if (num >= 1)
            {
                msg = ReleaseLayerOfOnionAnswer(msg, client.entryNode.key);
            }
            if (num >= 2)
            {
                msg = ReleaseLayerOfOnionAnswer(msg, client.middleNode.key);
            }
            if (num >= 3)
            {
                msg = ReleaseLayerOfOnionAnswer(msg, client.exitNode.key);
            }

            JObject j = deserilaizeToObject(msg);

            return j;
        }

        /*
         * Function that release layer of onion by the key from data
         * input: data to release layer from, aes key
         * output: decrypted data by aes key
         */
        static private String ReleaseLayerOfOnionAnswer(String data, byte[] key)
        {
            JObject j = deserilaizeToObject(data);

            String enc = j["reply"].ToString();

            Console.WriteLine("key: " + key.ToString());

            String dec = Aes.DecryptFromBase64(enc, Convert.ToBase64String(key));

            return dec;
        }

        /*
         * Function that do handshake with the nodes
         * input: client
         * output: none
         */
        static public void handshake(Client client)
        {
            // Entry node key

            client.entryNode.key = handshakeWithOneNode(client, 0);

            Console.WriteLine("handshake with entry node finish: {client.entry_node.key}");

            // --------------------------------------------------------------------------------------------------- #

            // Middle node key

            client.middleNode.key = handshakeWithOneNode(client, 1);

            Console.WriteLine("handshake with middle node finish: {client.middle_node.key}");

            // --------------------------------------------------------------------------------------------------- #

            // Exit node key

            client.exitNode.key = handshakeWithOneNode(client, 2);

            Console.WriteLine("handshake with exit node finish: {client.exit_node.key}");

            // --------------------------------------------------------------------------------------------------- #

            Console.WriteLine("Handshake done!\n");
        }

        /*
         * Function that validate the server certificate
         * input:
         * output:
         */
        public static bool ValidateServerCertificate(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors sslPolicyErrors)
        {
            var serverCert = new X509Certificate2(certificate);

            var customChain = new X509Chain();

            customChain.ChainPolicy.TrustMode = X509ChainTrustMode.CustomRootTrust;

            // load the crt file
            var caCert = new X509Certificate2("ca.crt");
            customChain.ChainPolicy.CustomTrustStore.Add(caCert);

            customChain.ChainPolicy.RevocationMode = X509RevocationMode.NoCheck;
            customChain.ChainPolicy.VerificationFlags = X509VerificationFlags.NoFlag;

            bool isValid = customChain.Build(serverCert);

            if (!isValid)
            {
                foreach (var status in customChain.ChainStatus)
                {
                    System.Diagnostics.Debug.WriteLine($"Chain error: {status.Status}");
                    System.Diagnostics.Debug.WriteLine(status.StatusInformation);
                }
            }

            return isValid;
        }

        /*
         * Function that send object through entry node of client
         * input: object, client, request code
         * output: none
         */
        static public void sendMsg<T>(T obj, Client client, int msgCode)
        {
            String json = "", encryptedMsg = "";

            json = JsonConvert.SerializeObject(obj, Formatting.Indented);

            String msg = (char)(msgCode + 48) + json;

            if(!_isFirstMessage)
            {
                encryptedMsg = buildOnionWithoutPorts(msg, client);
            }
            else
            {
                _isFirstMessage = false;
                encryptedMsg = buildOnionWithPorts(msg, client);
            }

            sendMsg(client.entryNodeSock, encryptedMsg);
        }

        /*
         * Function that send object through entry node of client and return the answer of the server
         * input: object, client, request code
         * output: the answwer of the server
         */
        static public JObject sendMsgAndReceiveAnswer<T>(T obj, Client client, int msgCode)
        {
            sendMsg(obj, client, msgCode);

            JObject result;

            while (!_queue.TryDequeue(out result))
            {
                Thread.Sleep(10);
            }

            return result;
        }

        /*
         * Function that release the layers from the response of the server
         * input: encrypted replay, client
         * output: the decrypted response
         */
        private static String PeelReply(string rawReply, Client client)
        {
            // JSON {"reply": <encrypted>}
            String current = rawReply;

            byte[] key1 = client.entryNode.key;
            byte[] key2 = client.middleNode.key;
            byte[] key3 = client.exitNode.key;

            byte[][] keys = { key1, key2, key3 };

            JObject j = null;

            foreach (byte[] key in keys)
            {
                current = ReleaseLayerOfOnionAnswer(current, key);
            }

            return current;
        }

        /*
         * The function is received messages from the server (through the entry node) and enter them to the queue. If there is error response code, message box will show with the error message
         * input: client
         * output: none
         */
        static public void receiveAnsThread(Client client)
        {
            String answer = "";
            String lengthAnswer = "";
            JObject ansObj;
            char id = ' ';
            uint idUint = 0;
            byte[] firstFiveBytes = new byte[6];

            try
            {
                while (true)
                {
                    // get encrypt msg
                    answer = receiveMsg(client.entryNodeSock);

                    answer = PeelReply(answer, client);

                    id = answer[0];
                    idUint = (uint)id;

                    if (answer.Length > 10)
                    {
                        answer = answer.Substring(1);
                        ansObj = deserilaizeToObject(answer);
                    }
                    else
                    {
                        System.Windows.Application.Current.Dispatcher.BeginInvoke(() => MessageBox.Show("Answer is empty!!!"));
                        continue;
                    }

                    if(idUint != 111 && idUint != 113)
                    {
                        _queue.Enqueue(ansObj);
                    }

                    if(idUint == 113) // receive message from another user
                    {
                        App.MenuWindow?.addNewConversation(deserilaizeToObject(answer));
                    }
                    else if (idUint == 111) // receive message from another user
                    {
                        App.MenuWindow?.receiveMessage(deserilaizeToObject(answer));
                    }
                    else if (idUint == 99) // error after illegal request
                    {
                        if (ansObj["message"] != null)
                        {
                            System.Windows.Application.Current.Dispatcher.BeginInvoke(() => MessageBox.Show(ansObj["message"].ToString()));
                        }
                    }

                    lengthAnswer = "";
                }
            }
            catch (IOException ex) when (ex.InnerException is SocketException)
            {
                System.Diagnostics.Debug.WriteLine("socket closed");
            }
        }

        static public JObject deserilaizeToObject(string str)
        {
            JObject obj = JObject.Parse(str);
            return obj;
        }
    }
}