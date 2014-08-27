#include "boost/graph/adjacency_list.hpp"
#include <boost/graph/graphviz.hpp> // Good for debugging, but take out for final build.
#include "boost/graph/breadth_first_search.hpp"
#include "CheckPointGraph.hpp"
#include <boost/bimap/bimap.hpp>
#include <boost/bimap/set_of.hpp>

using namespace boost; // Useful for graphs

/** Array of the names of checkpoints. Necessary for the initialisation of the checkpoints vector. */
const char* nameArr[] = { 
		"FrontDoorWest","FrontDoorEast","LivingRoomNorthWest","LivingRoomNorthEast","CentrePassageSouth","CentrePassageNorth",
		"KitchenNorthWest","KitchenSouthWest","KitchenSouthEast","KitchenNorthEast","HouseCentre","CentreStool","NextToCentreStool",
		"BedroomEntranceWest","BedroomEntranceEast","CouchesNorthEast","CouchesNorthCentre","BathroomEntranceWest","BathroomEntranceEast",
		"Shower","BathroomCentre","BedSouthWest","BedSouthEast","BedNorthEast","ResidentOrigin", "Assistant1Origin", "Assistant2Origin",
		"DoctorOrigin","Nurse1Origin","Nurse2Origin","CaregiverOrigin","Friend1Origin","Friend2Origin","Friend3Origin"
};

/**
*	@brief Set of checkpoints that the nodes can move to in the map.
* 	Gives x position and y position for each co-ordinate
*/
int checkpoints[][2] = {
	{-27,-40},{-20,-40},{-24,-12},{-18,-18},{0,-18},{0,-12},{6,-24},{6,-28},{24,-28},{24,-24},{0,6},{24,-10},{20,-6},{0,22},
	{6,22},{-6,18},{-24,18},{-26,22},{-18,22},{-32,45},{-24,36},{6,30},{26,30},{30,45},
	{26,48}, {32,20}, {32,18}, {-33,-46}, {-36,-48}, {-30,-48}, {-8,-46}, {-20,-46}, {-23,-46}, {-20,-48}
};

std::vector<std::string> checkpointNames(begin(nameArr), end(nameArr)); /*!< Vector of checkpoint names. See nameArr[]. */

/* -- Graph -- */

typedef property<vertex_name_t, std::string> VertexProperty; /*!<  Will allow us to retrieve vertex names from vertex references */
typedef adjacency_list <vecS, vecS, undirectedS, VertexProperty> vector_graph_t; /*!< Graph of checkpoint names */
const int checkpointNum = checkpointNames.size(); /*!< Number of checkpoints. */
vector_graph_t g(checkpointNum); // Our map
std::map<std::string, vector_graph_t::vertex_descriptor> indices; /*!< Map that corresponds checkpoint names to the vertices in the graph. */

/* -- Edges -- */

typedef std::pair <std::string, std::string> E;
E paths[] = {
	 E("FrontDoorWest","LivingRoomNorthWest"), E("FrontDoorEast", "LivingRoomNorthEast"), E("LivingRoomNorthWest","CentrePassageNorth"),
	 E("LivingRoomNorthEast","CentrePassageSouth"), E("CentrePassageNorth","KitchenNorthWest"), E("CentrePassageSouth","KitchenNorthWest"),
	 E("KitchenNorthWest","KitchenNorthEast"), E("KitchenNorthEast","KitchenSouthEast"), E("KitchenSouthEast","KitchenSouthWest"),
	 E("KitchenSouthWest","KitchenNorthWest"), E("CentrePassageSouth","NextToCentreStool"), E("CentrePassageNorth","NextToCentreStool"),
	 E("NextToCentreStool","CentreStool"), E("NextToCentreStool","HouseCentre"), E("CentrePassageNorth","HouseCentre"),
	 E("CentrePassageNorth","HouseCentre"), E("HouseCentre", "CouchesNorthEast"), E("CouchesNorthEast","CouchesNorthCentre"),
	 E("CouchesNorthEast","BedroomEntranceEast"), E("CouchesNorthEast","BedroomEntranceWest"), E("CouchesNorthCentre","BathroomEntranceEast"),
	 E("CouchesNorthCentre","BedroomEntranceWest"), E("BathroomEntranceEast","BathroomCentre"), E("BathroomEntranceWest","BathroomCentre"),
	 E("BathroomCentre","Shower"), E("BedroomEntranceWest","BedSouthWest"), E("BedroomEntranceEast","BedSouthEast"), 
	 E("BedSouthEast","BedNorthEast"), E("Origin","BedSouthEast"),
	 E("DoctorOrigin","FrontDoorWest"), E("Nurse1Origin","FrontDoorWest"), E("Nurse2Origin","FrontDoorWest"),
	 E("Friend1Origin","FrontDoorEast"), E("Friend2Origin","FrontDoorEast"), E("Friend3Origin","FrontDoorEast"),
	 E("CaregiverOrigin","FrontDoorEast"),
	 E("Assistant1Origin","CouchesNorthEast"), E("Assistant2Origin","CouchesNorthEast"), E("ResidentOrigin","BedNorthEast"),
	 E("HouseCentre","CentrePassageSouth")
}; /*!< Defines edges between checkpoints */

/* -- Map of names to co-ordinates -- */

// // typedef std::string CheckpointName; // Key
// // typedef std::pair<int, int> Checkpoint; // Value
// typedef boost::bimaps::bimap< boost::bimaps::set_of<std::string>, boost::bimaps::set_of<std::pair<int, int> > > CheckpointMap;
// CheckpointMap c;

// typedef CheckpointMap::left_map CheckpointNames;
// CheckpointNames& names = c.left;
// typedef CheckpointNames::value_type CheckpointName;
// typedef CheckpointNames::const_iterator CheckpointNamesIterator;

// typedef CheckpointMap::right_map CheckpointCoords;
// CheckpointCoords& coords = c.right;
// typedef CheckpointCoords::value_type CheckpointCoord;
// typedef CheckpointCoords::const_iterator CheckpointCoordsIterator;

/* -- Temporary solution - two maps of co-ordinates to names and names to co-ordinates --*/

typedef std::string CheckpointName; // Key
typedef std::pair<int, int> Checkpoint; // Value
typedef std::map<CheckpointName, Checkpoint> CheckpointMap; /*!< Map with checkpoint names as keys and checkpoint co-ordinates as values */
CheckpointMap c;

typedef std::map<Checkpoint, CheckpointName> CheckpointMapReverse; /*!< Map with checkpoint names as keys and checkpoint co-ordinates as values */
CheckpointMapReverse crev;

/* -- Pathing -- */

int cc; /*!< The index of the agent's current checkpoint in the path. */
std::vector<std::pair<double, double> > path; /*!< The agent's path to a specified goal. */

/**
 *	@brief Finds the shortest path between 2 checkpoints, and returns the path as co-ordinates.
 *	Uses breadth first search - Boost recommends this over Dijkstra's algorithm for graphs with uniformly weighted edges.
 * 	Sets path member variable.
 *	@param startName The name of the start checkpoint as a string (e.g. 'kitchen')
 *	@param endName The name of the goal checkpoint as a string (e.g. 'bathroom')
 */
std::vector<std::pair<double, double> > CheckPointGraph::shortestPath(std::string startName, std::string endName) {
	CheckPointGraph::checkpointMap();
	CheckPointGraph::makeGraph();

	//Create vector to store the predecessors (can also make one to store distances)
  	std::vector<vector_graph_t::vertex_descriptor> p(boost::num_vertices(g));

  	// Get the descriptor for the source node
  	vector_graph_t::vertex_descriptor s = indices[startName]; 

 	// Computes the shortest path 
 	breadth_first_search(g, s, visitor(make_bfs_visitor(record_predecessors(&p[0], on_tree_edge()))));

 	// Get the path back from the predecessor map
 	vector_graph_t::vertex_descriptor goal = indices[endName]; // As above, end checkpoint
 	std::vector<vector_graph_t::vertex_descriptor> path;
	vector_graph_t::vertex_descriptor current;

	current = goal;

	// This loop could be eliminated by pushing checkpoint names to a vector rather than vertex_descriptors. However,
	// I left it in because it seemed like good practice to create/retain the actual path of vertices, e.g. in case
	// there we should implement and want to access properties other than just vertex names.
	while(current!=s) {
   		path.push_back(current);
    	current = p[current]; // Predecessor of the current checkpoint in the path
	}

	path.push_back(s); // BFS doesn't include the start node. 

	std::vector<std::pair<double, double> > a;

	for (int i=0; i<path.size(); i++) {
		// Get the vertex name from the graph's property map
		std::string cpn = boost::get(vertex_name_t(), g, path[i]); // adjacency_list vertex_descriptors are ints
		std::pair<double, double> coords = c.at(cpn); // Get co-ordinates associated with checkpoint name
		a.push_back(coords);
	}
	
	std::reverse(a.begin(), a.end()); // as search starts from goal; we can access only predecessors, not successors

    return a;
}

// checked
std::string CheckPointGraph::getCheckpointName(std::pair<double, double> cpcoords) {
	return crev.at(cpcoords);
}

/**
*	@brief Creates a graph of checkpoint names, provided the vector of names and the array of edges.
*	Uses boost's adjacency list.
*/ // checked
void CheckPointGraph::makeGraph() {

	// Fills the property 'vertex_name_t' of the vertices, allowing us to get the checkpoint name back when we have 
	// only a reference to the vertex (as will be the case when examining the shortest path). Also associates each
	// checkpoint name with a vertex descriptor.
	for(int i = 0; i < checkpointNum; i++)
	{
	  boost::put(vertex_name_t(), g, i, checkpointNames[i]); 
	  indices[checkpointNames[i]] = boost::vertex(i, g); 
	}

	// Add the edges. 
	for(int i = 0; i < sizeof(paths)/sizeof(paths[0]); i++)
	{
	  boost::add_edge(indices[paths[i].first], indices[paths[i].second], g);
	}

	// //Prints a pretty graph
	// std::ofstream ofs("test.dot");
    // write_graphviz(ofs, g); // dot -Tps test.dot -o outfile.ps	

}

/**
 *	@brief Associates checkpoint names with checkpoint co-ordinates
 *	To be used in conjunction with a graph of checkpoint names, representing paths between checkpoints. Could be replaced
 *	by adding the co-ordinates to bundled properties in the property map of the graph.
 */ //checked
void CheckPointGraph::checkpointMap() {

	// Convert array to pairs
	std::vector<std::pair<int,int> > vec;
	for (int i=0; i<checkpointNum; i++) {
		std::pair<int, int> p = std::make_pair(checkpoints[i][0],checkpoints[i][1]);
		vec.push_back(p);
	}

	// Add checkpoint name and checkpoint co-ordinates to the map
	for (int i=0; i<checkpointNum; i++) {
		c.insert(std::make_pair(CheckpointName(checkpointNames[i]), Checkpoint(vec[i])));
		crev.insert(std::make_pair(Checkpoint(vec[i]), CheckpointName(checkpointNames[i])));
	}

}
