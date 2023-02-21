#include <vector>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <unordered_map>

std::vector<bool> dec_to_bin_unsigned(int num, int bit_size)
{
	int temp_num = num;
	std::vector<bool> binary;
	for (int i = 0; i < bit_size; i++) {
		int mod = temp_num % 2;
		binary.push_back(mod);
		temp_num /= 2;
	}
	return binary;
}

int bin_to_dec_unsigned(std::vector<bool> binary)
{
	int base = 1;
	int dec = 0;
	for (int i = 0; i < binary.size(); i++) {
		dec += base*binary[i];
		base *= 2;
	}
	return dec;
}

std::vector<bool> dec_to_bin_signed(int num, int bit_size)
{
	int temp_num;
	if (num >= 0) {
		temp_num = num;
	} else {
		temp_num = pow(2, bit_size - 1) + num;
	}
	std::vector<bool> binary;
	for (int i = 0; i < bit_size - 1; i++) {
		int mod = temp_num % 2;
		binary.push_back(mod);
		temp_num /= 2;
	}
	binary.push_back(num < 0);
	return binary;
}

int bin_to_dec_signed(std::vector<bool> binary)
{
	int base = 1;
	int dec = 0;
	for (int i = 0; i < binary.size() - 1; i++) {
		dec += base*binary[i];
		base *= 2;
	}

	dec -= base*binary[binary.size() - 1];

	return dec;
}

struct Node {

private:
	int fanout;
	std::vector<Node*> children;

public:
	int msb;
	int lsb_connection; // the column to which it is connected
	int lsb;
	int depth;
	Node* msb_parent;
	Node* lsb_parent;

	Node(): msb_parent(nullptr), lsb_parent(nullptr), fanout(0) {}
	void add_child(Node* child) {
		fanout++;
		children.push_back(child);
	}

	std::vector<Node*> get_children() {return children;}

	void print() {
		std::cout << "=====================================" << std::endl;
		std::cout << "MSB: " << msb << std::endl;
		std::cout << "LSB: " << lsb << std::endl;
		std::cout << "LSB connection: " << lsb_connection << std::endl;
		std::cout << "Depth: " << depth << std::endl;
		if (msb_parent) {
			std::cout << "MSB parent: " << msb_parent->msb << " " << msb_parent->depth << std::endl; 
		} else {
			std::cout << "MSB bit: " << msb << std::endl; 
		}

		if (lsb_parent) {
			std::cout << "LSB parent: " << lsb_parent->msb << " " << lsb_parent->depth << std::endl; 
		} else {
			std::cout << "LSB bit: " << lsb << std::endl; 
		}

		std::cout << "Fanout: " << fanout << std::endl;
		std::cout <<  "Children: " << std::endl;
		for (Node* c : children) {
			std::cout << "Child: " << c->msb << " " << c->depth << std::endl; 
		}
		std::cout << "=====================================" << std::endl;
	}



};

class PrefixAdder {
private:
	int bit_num;
	std::vector<int> sequence;
	std::vector<Node*> node_sequence;
	std::vector<std::vector<Node*>> node_per_column;
	std::string sequence_str;
	
public:
	PrefixAdder(int bits, std::vector<int> seq) : bit_num(bits), sequence(seq), sequence_str("") {

		node_per_column.resize(bits);

		for (int i = 0; i < seq.size(); i++) {
			Node* n = new Node();
			n->msb = seq[i];
			node_sequence.push_back(n);
			// node_per_column[seq[i]].push_back(n);
			sequence_str += std::to_string(n->msb) + " ";
		}

		for (int i = 0; i < node_sequence.size(); i++) {
			Node* n = node_sequence[i];
			int msb = n->msb;

			// find msb
			Node* msb_parent = nullptr;
			int msb_depth = -1;
			if (node_per_column[msb].size() != 0) {
				msb_parent = node_per_column[msb][node_per_column[msb].size() - 1];
				msb_depth = msb_parent->depth;
			}

			// find lsb
			int lsb_connection = 0;
			int lsb = 0;
			Node* lsb_parent = nullptr; 
			int lsb_depth = -1;

			// find which column to connect your lsb to
			if (i != 0 && node_sequence[i - 1]->msb < n->msb) {
				// if npt the first node in the sequence, and the previous node has a smaller id, then connect to that node
				lsb_connection = node_sequence[i - 1]->msb;
				// std::cout << "lsb conn 1 " << lsb_connection << std::endl; 
			} else {
				if (!msb_parent) {  
					// if you have no msb, then you are connected to a bit. 
					// So your lsb should be the column right next to the msb
					lsb_connection = n->msb - 1;
					// std::cout << "lsb conn 2 " << lsb_connection << std::endl; 
				} else {
					// if you have an msb parent, connect to the column next to its lsb to preserve continuity
					lsb_connection = msb_parent->lsb - 1;
					// std::cout << "lsb conn 3 " << lsb_connection << std::endl; 
				}
			}

			// find your actual lsb parent
			if (node_per_column[lsb_connection].size() == 0) {
				// if there is no node to connect your lsb to, connect to the bit
				lsb = lsb_connection;
			} else {
				// else connect to the last node in that column 
				lsb_parent = node_per_column[lsb_connection][node_per_column[lsb_connection].size() - 1];
				lsb_depth = lsb_parent->depth;
				lsb = lsb_parent->lsb;
			}

			n->depth = std::max(lsb_depth, msb_depth) + 1;
			if (n->depth == 0) {
				n->depth++;
			}
			n->lsb = lsb;
			n->lsb_connection = lsb_connection;
			n->lsb_parent = lsb_parent;
			n->msb_parent = msb_parent;

			// update his parents
			if (n->lsb_parent) {
				n->lsb_parent->add_child(n);
			}
			if (n->msb_parent) {
				n->msb_parent->add_child(n);
			}

			node_per_column[seq[i]].push_back(n);

			// n->print();

		}
	}

	int calculate_sum(int dec_A, int dec_B, bool C_in=false, bool signed_inputs=true) {
		std::vector<bool> A = signed_inputs ? dec_to_bin_signed(dec_A, bit_num) : dec_to_bin_unsigned(dec_A, bit_num); 
		std::vector<bool> B = signed_inputs ? dec_to_bin_signed(dec_B, bit_num) : dec_to_bin_unsigned(dec_B, bit_num); 

		std::vector<bool> S(bit_num+1);

		std::vector<bool> p_bits(bit_num);
		std::vector<bool> g_bits(bit_num);

		bool p_0;
		bool g_0;

		std::unordered_map<Node*, bool> node_p;
		std::unordered_map<Node*, bool> node_g;

		for (int i = 0; i < bit_num; i++) {
			g_bits[i] = A[i] & B[i];
			p_bits[i] = A[i] ^ B[i];
		}

		// g_bits[0] = g_bits[0] | (p_bits[0] & C_in);

		for (Node* n : node_sequence) {
			bool p_msb, g_msb;
			if (n->msb_parent) {
				p_msb = node_p[n->msb_parent];
				g_msb = node_g[n->msb_parent];
			} else {
				p_msb = p_bits[n->msb];
				g_msb = g_bits[n->msb];
			}

			bool p_lsb, g_lsb;
			if (n->lsb_parent) {
				p_lsb = node_p[n->lsb_parent];
				g_lsb = node_g[n->lsb_parent];
			} else {
				p_lsb = p_bits[n->lsb];
				g_lsb = g_bits[n->lsb];
			}

			node_g[n] = g_msb | (p_msb & g_lsb);
			node_p[n] = p_msb & p_lsb;
		}

		S[0] = p_bits[0]; //^ C_in;

		for (int i = 1; i < bit_num; i++) {
			bool g_prev;
			if (i == 1) {
				g_prev = g_bits[0];
			} else {
				Node* last_node = node_per_column[i - 1][node_per_column[i - 1].size() - 1];
				g_prev = node_g[last_node];
			}
			S[i] = p_bits[i] ^ g_prev;
		}

		Node* last_node = node_per_column[bit_num - 1][node_per_column[bit_num - 1].size() - 1];
		bool g_prev = node_g[last_node];
		S[bit_num] = g_prev ^ A[bit_num - 1] ^ B[bit_num - 1];

		int S_dec = signed_inputs ? bin_to_dec_signed(S) : bin_to_dec_unsigned(S);
		return S_dec;

	}

	void debug() {
		std::cout << "Sequence string: " << sequence_str << std::endl;

		std::cout << "Sequence from vector: ";
		for (Node* n : node_sequence) {
			std::cout << n->msb << " ";
		}
		std::cout << std::endl;

		std::cout << "Nodes per column: " << std::endl;
		for (int i = 0; i < bit_num; i++) {
			std::cout << i << ": ";
			std::vector<Node*> v = node_per_column[i];
			for (Node* n : v) {
				std::cout << n->msb << " ";
			}
			std::cout << std::endl;
		}

		std::cout << "Print all nodes: " << std::endl;
		for (Node* n : node_sequence) {
			n->print();
		}
	}

	~PrefixAdder()
	{
		for (Node* n : node_sequence) {
			delete n;
		}
	}


};