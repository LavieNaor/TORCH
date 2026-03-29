using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Security;
using System.Net.Sockets;

namespace TorClient
{
    public class Client
    {
        // define the c'tor of each node
        public Node entryNode { get; }
        public Node middleNode { get; }
        public Node exitNode { get; }
        public SslStream entryNodeSock { get; set; }

        public Client(Node[] nodes)
        {
            // set the nodes
            this.entryNode = nodes[0];
            this.middleNode = nodes[1];
            this.exitNode = nodes[2];

            // init the socket
            entryNodeSock = null;
        }
    }
}
