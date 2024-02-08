import networkx as nx
import matplotlib.pyplot as plt
import json
import os
import time
import sys
import argparse


# If a directed cyclic graph is passed then it is converted to the DAG
# Using DFS the cycle is calculated from the root node and last (back) edge is removed until DAG is formed
# NOTE: Be aware that the backedge removed may not be the intended edge in the cycle
def dcg_to_dag(G, root_node = None):
	num_cycles = 0
	try:
		# print(f"Root node: {root_node}")
		while cycle_edges := list(nx.find_cycle(G, orientation='original', source=root_node)):
			# print(f"Cycle: {cycle_edges}")
			back_edge = cycle_edges[-1]
			G.remove_edge(back_edge[0], back_edge[1])
			# print(f"Removed back edge: {back_edge}")
			num_cycles += 1
	except nx.exception.NetworkXNoCycle:
		# print(f"DAGed after {num_cycles} cycles")
		assert nx.is_directed_acyclic_graph(G)
		# nx.draw_networkx(G)
		plt.show()

def top_longest_paths(G, root_node, cutoff=20):
	all_paths=[]
	assert root_node
	i = 1

	print(f"Total nodes: {len(G.nodes)}")
	for node in G.nodes:
		print(f"{i}.{root_node}->{node}")
		i += 1

		if node != root_node:
			for path in nx.all_simple_paths(G, root_node, node):
				all_paths.append(path)

	all_paths.sort(key=len,reverse=True)
	return all_paths[:cutoff]


# def write_depths_data(deep_paths, filename="deep_stacks.json", root_node=None):
# 	assert deep_paths
# 	assert deep_paths[0]
# 	assert deep_paths[0][0]

# 	if root_node != None:
# 		assert deep_paths[0][0] == root_node
# 	else:
# 		root_node = deep_paths[0][0]

# 	json_obj = {root_node: {"max_depth_count": len(deep_paths[0]),
# 				"deep_paths": deep_paths}}

# 	print(stack_depth_json_file_obj)
# 	stack_depth_json_file_obj.append(json_obj)
# 	print(stack_depth_json_file_obj)

# 	with open(filename, "w") as f:
# 		json.dump(stack_depth_json_file_obj, f)

def write_path_data(path, known_frames_total_size, known_frame_lens_count, filename):
	assert path
	assert path[0]
	assert filename

	if os.path.exists(filename):
		with open(filename, "r") as f:
			stack_depth_json_file_obj = json.load(f)
	else:
		print("No out JSON file found at {filename}")
		return

	stack_depth_json_file_obj = []
	json_obj = {path[0]: {"max_depth": len(path),
				"known_frames_total_size": known_frames_total_size,
				"known_frame_lens_count": known_frame_lens_count,
				"max_depth_path": path}}

	# print(stack_depth_json_file_obj)
	stack_depth_json_file_obj.append(json_obj)
	# print(stack_depth_json_file_obj)

	with open(filename, "w") as f:
		json.dump(stack_depth_json_file_obj, f)


def load_edges(root_node: str):
	filename = root_node + ".txt"
	edges = []

	assert os.path.exists(filename)

	with open(filename, "r") as f:
		for line in f:
			line = line.strip().split()  # Split the line into a list of strings
			for i in range(1, len(line)):
				edges.append((line[0], line[i]))

	return edges

def get_stack_frame_lengths(func_stack_sizes, stacklens_file=None):
	# Open the file for reading
	

	with open(stacklens_file, 'r') as file:
	    # Iterate over each line in the file
	    for line in file:
	        # Split the line into two parts around colon
	        parts = line.split(':')
	        if len(parts) != 2:
	            continue  # Ignore lines that don't contain a colon

	        # Extract the function name and value from the line
	        function_name = parts[0].split()[1]
	        value_str = parts[1].strip()

	        # Parse the value as an integer
	        try:
	            value = int(value_str)
	        except ValueError:
	            continue  # Ignore lines where the value is not an integer

	        func_stack_sizes[function_name] = value
	        # Print the extracted information
	        #print(f"Function name: {function_name}, Value: {value}")

	# print(func_stack_sizes)


def find_max_depth(func_name, out_json_file=None, stacklens_file=None):
	# Load all functions from the .txt file and create a list of edges
	edges = load_edges(func_name)

	# Create a directed graph with a cycle
	G = nx.DiGraph()
	G.add_edges_from(edges)

	# If the graph generated is Cyclic Graph then convert it to DAG
	dcg_to_dag(G, func_name)

	# Get the longest path and check if the path is from the func_name
	path = nx.dag_longest_path(G)
	assert path[0] == func_name
	path_len = len(path)

	func_stack_sizes = {}
	get_stack_frame_lengths(func_stack_sizes, stacklens_file)



	print(f"Function: \033[34m{func_name}\033[0m")
	print(f"Max stack functions depth: \033[31m{path_len}\033[0m")
	print(f"Max stack functions path:")
	known_frame_lens_count = 0
	known_frames_total_size = 0
	for path_fn in path:
		if frame_len := func_stack_sizes.get(path_fn):
			known_frame_lens_count += 1
			known_frames_total_size += frame_len

		print(f"->\033[32m{path_fn}\033[0m(\033[31m{frame_len if frame_len else '?'}\033[0m)", end="")
	print("\n")

	print(f"Known max stack depth size: \033[31m{known_frames_total_size}\033[0m bytes ({known_frame_lens_count} of {path_len})")

	if out_json_file:
		write_path_data(path, known_frames_total_size, known_frame_lens_count, out_json_file)


if __name__ == "__main__":
	parser = argparse.ArgumentParser()
	parser.add_argument("function", type=str, help="Helper function or File with compiler output")
	parser.add_argument("--stacklen", type=str, help="Stack frame sizes out file from cheakstack.pl script")
	parser.add_argument("--out", type=str, help="Out JSON file")
	args = parser.parse_args()

	func = args.function
	out_json_file = args.out

	if args.stacklen:
		stacklens_file = args.stacklen
	else:
		assert os.path.exists("./stack_sizes.txt")
		stacklens_file = "./stack_sizes.txt"


	if func.endswith(".txt"):
		func = func[:len(func) - len(".txt")]

	find_max_depth(func, out_json_file, stacklens_file) 
