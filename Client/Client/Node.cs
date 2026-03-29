using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TorClient
{
    public class Node
    {
        public String ip { get; set; }
        public int port { get; set; }
        public byte[] key { get; set; }

        public Node(String ip, int port)
        {
            this.ip = ip;
            this.port = port;
        }
    }
}
