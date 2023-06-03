from fmm import Network

net1 = Network('example/data/edges.shp')
assert net1.dump('network.json')

net2 = Network()
assert net2.load('network.json')
assert net2.dump('network_redump.json')
