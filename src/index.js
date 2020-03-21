const node_rand = require("./node_rand.node");
const { Readable } = require('stream')
node_rand.SetReadable(Readable);

exports.NodeRand = node_rand.NodeRand;