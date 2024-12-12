# DataVis App

## About
The app can parse .dot files into graphs, MSP tree (minimum spanning tree using Prim's algorithm) and clusters.

The following visualization algorithms are implemented:
- Random (Default)
- Grid
- T-SNE
- MDS
- Force directed
- Radial (Only tree's)
- Sugiyama (Only graphs)
- Edge bundling (Only clusters)

[Demo video](https://youtu.be/ND90xi-xi3g?si=VcY4xksNrb0VVZS5)

*Created by Tariq Bakhtali & Merijn Schepers*

## Dependencies
- OpenFrameworks (0.12)
- Boost

## Setup
- Download openFrameworks
- Put the contents of `apps/` folder into the openFrameworks `apps` folder
- Put the contents of `libs/` folder into the openFrameworks `libs` folder
- You can find the soluation in `apps\data-vis\`

## Shortcomings
- Cannot converse graphs into tree's and vice-versa.
- Bloated; the .dot parser depends on Boost.
