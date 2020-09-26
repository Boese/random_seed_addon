const node_rand = require("./node_rand.node");
const { Readable } = require('stream')
node_rand.NodeRand_mt19937.SetReadable(Readable);
node_rand.NodeRand_mt19937_64.SetReadable(Readable);

exports.NodeRand_mt19937 = node_rand.NodeRand_mt19937;
exports.NodeRand_mt19937_64 = node_rand.NodeRand_mt19937_64;