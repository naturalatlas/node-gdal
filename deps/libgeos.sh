#!/bin/bash

set -eu

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$DIR/libgeos"

GEOS_VERSION=3.8.0
dir_geos=./geos

#
# download geos source
#

rm -rf $dir_geos
if [[ ! -f geos-${GEOS_VERSION}.tar.bz2 ]]; then
	curl -L https://download.osgeo.org/geos/geos-${GEOS_VERSION}.tar.bz2 -o geos-${GEOS_VERSION}.tar.bz2
fi
tar -xzf geos-${GEOS_VERSION}.tar.bz2
mv ./geos-${GEOS_VERSION} $dir_geos

#
# apply patches
#

# Fix for error:
# > static library deps/libgeos/libgeos.gyp:libgeos#target has several files with the same basename:
# >   NodeBase: geos/src/index/bintree/NodeBase.cpp geos/src/index/quadtree/NodeBase.cpp
# >   DirectedEdgeStar: geos/src/geomgraph/DirectedEdgeStar.cpp geos/src/planargraph/DirectedEdgeStar.cpp
# >   NodeMap: geos/src/geomgraph/NodeMap.cpp geos/src/planargraph/NodeMap.cpp
# >   Key: geos/src/index/bintree/Key.cpp geos/src/index/quadtree/Key.cpp
# >   SweepLineEvent: geos/src/geomgraph/index/SweepLineEvent.cpp geos/src/index/sweepline/SweepLineEvent.cpp
# >   Root: geos/src/index/bintree/Root.cpp geos/src/index/quadtree/Root.cpp
# >   EdgeRing: geos/src/geomgraph/EdgeRing.cpp geos/src/operation/polygonize/EdgeRing.cpp
# >   DirectedEdge: geos/src/geomgraph/DirectedEdge.cpp geos/src/planargraph/DirectedEdge.cpp
# >   Node: geos/src/geomgraph/Node.cpp geos/src/planargraph/Node.cpp geos/src/index/bintree/Node.cpp geos/src/index/quadtree/Node.cpp
# >   Interval: geos/src/index/bintree/Interval.cpp geos/src/index/strtree/Interval.cpp
# >   Edge: geos/src/geomgraph/Edge.cpp geos/src/planargraph/Edge.cpp
# >   PlanarGraph: geos/src/geomgraph/PlanarGraph.cpp geos/src/planargraph/PlanarGraph.cpp
# > Some build systems, e.g. MSVC08, cannot handle that.

mv $dir_geos/src/index/bintree/NodeBase.cpp $dir_geos/src/index/bintree/NodeBase_bintree.cpp
mv $dir_geos/src/index/quadtree/NodeBase.cpp $dir_geos/src/index/bintree/NodeBase_quadtree.cpp
mv $dir_geos/src/geomgraph/DirectedEdgeStar.cpp $dir_geos/src/geomgraph/DirectedEdgeStar_geomgraph.cpp
mv $dir_geos/src/planargraph/DirectedEdgeStar.cpp $dir_geos/src/planargraph/DirectedEdgeStar_planargraph.cpp
mv $dir_geos/src/geomgraph/NodeMap.cpp $dir_geos/src/geomgraph/NodeMap_geomgraph.cpp
mv $dir_geos/src/planargraph/NodeMap.cpp $dir_geos/src/planargraph/NodeMap_planargraph.cpp
mv $dir_geos/src/index/bintree/Key.cpp $dir_geos/src/index/bintree/Key_bintree.cpp
mv $dir_geos/src/index/quadtree/Key.cpp $dir_geos/src/index/quadtree/Key_quadtree.cpp
mv $dir_geos/src/geomgraph/index/SweepLineEvent.cpp $dir_geos/src/geomgraph/index/SweepLineEvent_geomgraph.cpp
mv $dir_geos/src/index/sweepline/SweepLineEvent.cpp $dir_geos/src/index/sweepline/SweepLineEvent_index.cpp
mv $dir_geos/src/index/bintree/Root.cpp $dir_geos/src/index/bintree/Root_bintree.cpp
mv $dir_geos/src/index/quadtree/Root.cpp $dir_geos/src/index/quadtree/Root_quadtree.cpp
mv $dir_geos/src/geomgraph/EdgeRing.cpp $dir_geos/src/geomgraph/EdgeRing_geomgraph.cpp
mv $dir_geos/src/operation/polygonize/EdgeRing.cpp $dir_geos/src/operation/polygonize/EdgeRing_polygonize.cpp
mv $dir_geos/src/geomgraph/DirectedEdge.cpp $dir_geos/src/geomgraph/DirectedEdge_geomgraph.cpp
mv $dir_geos/src/planargraph/DirectedEdge.cpp $dir_geos/src/planargraph/DirectedEdge_planargraph.cpp
mv $dir_geos/src/geomgraph/Node.cpp $dir_geos/src/geomgraph/Node_geomgraph.cpp
mv $dir_geos/src/planargraph/Node.cpp $dir_geos/src/planargraph/Node_planargraph.cpp
mv $dir_geos/src/index/bintree/Node.cpp $dir_geos/src/index/bintree/Node_bintree.cpp
mv $dir_geos/src/index/quadtree/Node.cpp $dir_geos/src/index/quadtree/Node_quadtree.cpp
mv $dir_geos/src/index/bintree/Interval.cpp $dir_geos/src/index/bintree/Interval_bintree.cpp
mv $dir_geos/src/index/strtree/Interval.cpp $dir_geos/src/index/strtree/Interval_strtree.cpp
mv $dir_geos/src/geomgraph/Edge.cpp $dir_geos/src/geomgraph/Edge_geomgraph.cpp
mv $dir_geos/src/planargraph/Edge.cpp $dir_geos/src/planargraph/Edge_planargraph.cpp
mv $dir_geos/src/geomgraph/PlanarGraph.cpp $dir_geos/src/geomgraph/PlanarGraph_geomgraph.cpp
mv $dir_geos/src/planargraph/PlanarGraph.cpp $dir_geos/src/planargraph/PlanarGraph_planargraph.cpp
