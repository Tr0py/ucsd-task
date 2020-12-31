#include<vector>
#include<map>
#include<iostream>
#include<iterator>
#include<algorithm>
#include<chrono>
#include<fstream>
#include <sstream>
#include <omp.h>
#include "svec.h"

typedef unsigned int SID;
typedef unsigned int Intensity;
typedef unsigned int MZ;

typedef std::pair<MZ, Intensity> Peak;
typedef std::vector<Peak> Spectrum;
typedef std::vector<Spectrum> RawData;

/* thread safe */

typedef std::pair<SID, Intensity> BucketPeak;

typedef std::vector<BucketPeak> Bucket;

typedef std::vector<Bucket> Index;

typedef svector<Peak> sSpectrum;
typedef std::map<SID, Spectrum> QueryResult;
typedef std::vector<QueryResult> QueryResults;


static const MZ MAX_MZ = 20000;
static const int num_buckets = 1; //# of bins per mz used for index


RawData * load_raw_data(char *file, int &total_spectra, int &num_peaks) {
	RawData * spectra = new RawData();
	unsigned int file_id;
	unsigned int num_spectra;           //total number of spectra inside the file
	std::vector< std :: pair<unsigned int, unsigned int> > position; // starting position and ending position for each spectrum
	std::ifstream in(file, std::ios::in | std::ios::binary);
	in.read((char*)&file_id, sizeof( unsigned int ));
	in.read((char*)&num_spectra, sizeof( unsigned int ));
	position.resize(num_spectra);

	//total_spectra = 0 as input means to load all spectra from the DB
	if (total_spectra == 0){
		total_spectra = num_spectra;
	}
    
	if (num_spectra > 0) {
		position[0].first = num_spectra + 2; //starting offset in the file of the first spectrum
		in.read((char *) &position[0].second, sizeof(unsigned int)); //ending position of the first spectrum

		for (unsigned int spec_idx = 1; spec_idx < num_spectra; ++spec_idx) {
			position[spec_idx].first = position[spec_idx - 1].second;  //starting position
			in.read((char *) &position[spec_idx].second, sizeof(unsigned int)); //ending position
		}

		for (unsigned int spec_idx = 0; spec_idx < total_spectra; ++spec_idx) {
			in.seekg(position[spec_idx].first * 4 + 16); //setting the offset to read the spectrum
			unsigned int size = position[spec_idx].second - position[spec_idx].first - 4; //total number of peaks inside the spectrum

			//num_peaks = 0 as input means to load all peaks of the spectrum
			if (num_peaks == 0 || num_peaks < size){
				num_peaks = size;
			}

			Spectrum spectrum;
			//Populating peak info per spectrum
			for (int i = 0; i < num_peaks; ++i) {
				unsigned int peak;
				unsigned int mz;
				in.read((char *) &peak, sizeof(unsigned int));
				mz = peak >> 8;
				spectrum.push_back(Peak(mz,peak - (mz<<8)));
			}
			spectra -> push_back(spectrum);
		}
	}
	in.close();
	return spectra;
}

void dump_spectrum(Spectrum *s) {
	int mz_count = 0, mz_max = 0;
	std::cerr << "[";
	for(auto & p: *s) {
		std::cerr << "[" << p.first << ", " << p.second << "],";
		mz_count++;
		if (p.first > mz_max) mz_max = p.first;
	}
	std::cerr << "]";
	printf("\nMZ count: %d, max: %d\n", mz_count, mz_max);
}

void dump_raw_data(RawData* r) {

	int sid_count = 0, sid_max = 0;
	int c = 0;
	for(auto & s: *r) {
		std::cerr << "SID=" << c;
		sid_count++;
		if (c > sid_max) sid_max = c;
		dump_spectrum(&s);
		c++;
		std::cerr << "\n";
	}
	//printf("SID count: %d, max: %d\n", sid_count, sid_max);
}

void dump_index(Index *index) {
	for(MZ mz = 0; mz < MAX_MZ; mz++) {
		if(!(*index)[mz].empty()) {
			std::cerr << "MZ = " << mz << ": ";
			dump_spectrum(&(*index)[mz]);
			std::cerr << "\n";
		}
	}
}


void json_reconstruction(char * file, const QueryResults &reconstructed_spectra) {

	std::ofstream out(file, std::ios::out);
	

	//auto end = reconstructed_spectra.end();
	out << "[\n";
	//for(const auto & queries_results: reconstructed_spectra) {
	int spectrum_max_size = 0;
		int first = 1;
	for (int i=0; i<reconstructed_spectra.size(); i++) {
		const QueryResult &queries_results = (reconstructed_spectra[i]);
		// Printf ',' before []   except the first one.
		if (first == 0) {
			out << ",";
		}
		out << "[\n";
		for(const auto &[sid, spectrum]:queries_results) {
			
			out << "\t{ " << "";
			out << "\"" << sid << "\": " << "[\n";
			//printf("spectrum len: %d\n", spectrum.size());
			if (spectrum.size() >  spectrum_max_size) spectrum_max_size = spectrum.size();
			//for (auto j = spectrum.begin(); j != spectrum.end(); j++) {
			for (int j=0; j<spectrum.size(); j++) {
				const Peak &peak = spectrum[j];
				out << "\t\t\t[ " << peak.first << ", " << peak.second << " ]";
				if (j != spectrum.size()-1) {
					out<<  ",";
				}
				/*
				out << "\t\t\t[ " << j->first << ", " << j->second << " ]";
				if (std::next(j) != spectrum.end()) {
					out<<  ",";
				}
				*/
				out << "\n";
			}
			out << "\t\t]\n";
			out << "\t}";
			
			if (&sid != &queries_results.rbegin()->first) {
				out << ",";
			}

			out << "\n";
		}
		out << "]";
		first = 0;
		out << "\n";
	}
	out << "]\n";
	printf("spectrum_max_size: %d\n", spectrum_max_size);
}


Spectrum * load_query(char*file) {
	Spectrum *n = new Spectrum;
	std::ifstream in(file);
	std::string line;

	while (std::getline(in, line)) {
		std::vector<unsigned int> lineData;
		std::stringstream lineStream(line);
		unsigned int value;
		while (lineStream >> value) {
			lineData.push_back(value);
		}
		n->push_back(Peak(lineData[0], lineData[1]));
	}
	return n;
}

std::vector<Spectrum> * load_queries(char*file) {
	auto n = new std::vector<Spectrum>;
	
	std::ifstream in(file);
	std::string line;

	int spectra_count = 0;
	in >> spectra_count;
	for(int i = 0; i < spectra_count; i++) {
		Spectrum s;
		int peak_count;
		in >> peak_count;
		for (int p = 0; p < peak_count; p++) {
			MZ mz;
			Intensity intensity;
			in >> mz >> intensity;
			s.push_back(Peak(mz, intensity));
		}
		n->push_back(s);
	}
	return n;
}

Index * build_index(RawData * data) {
	Index *index = new Index;

	for(MZ mz = 0; mz < MAX_MZ; mz++) {
		index->push_back(Bucket());
	}

	unsigned int unit_frag;

	int min_mz = 20000;
	for(SID sid = 0; sid < data->size(); sid++) {
		for(auto & peak: (*data)[sid]) {
			unit_frag = peak.first/num_buckets;
			if (unit_frag < MAX_MZ) {
				(*index)[unit_frag].push_back(BucketPeak(sid, peak.second));
				if (peak.first < min_mz) min_mz = peak.first;
			}
		}
	}
	printf("min_mz %d\n", min_mz);

	for(MZ mz = 0; mz < MAX_MZ; mz++) {
		std::sort((*index)[mz].begin(), (*index)[mz].end());
	}

	return index;
}

QueryResults* reconstruct_candidates(Index * index, const std::vector<Spectrum> & queries, QueryResults* query_results) {

#pragma omp parallel for
	for (int iquery=0; iquery<queries.size(); iquery++) {
		const Spectrum &query = queries[iquery];

		QueryResult *m = &((*query_results)[iquery]);
		{
			for (int i = 0; i < query.size(); i++) {
				unsigned int mz = query[i].first;
				for (int j = 0; j < (*index)[mz].size(); j++) {
					BucketPeak bucket_peak = (*index)[mz][j];
					//printf("mz %u, bucket size = %lu, bucketpeak.first %d\n", mz, (*index)[mz].size(), bucket_peak.first);
					//printf("i = %d, I am Thread %d\n", i, omp_get_thread_num());

//#pragma omp critical
					(*m)[bucket_peak.first].push_back(Peak(mz, bucket_peak.second));
				}
			}
		}
	}

	return query_results;
}

QueryResults* init_query_results(unsigned int size)
{
	QueryResults *qr = new QueryResults(size);
	//for (auto map : (*qr)) {
	//}
	return qr;
}

int main(int argc, char * argv[]) {

	if (argc != 4) {
		std::cerr << "Usage: main <raw data file> <query file> <output json>\n";
		exit(1);
	}

	/* Declare number of spectra you want to load from the database
	 * 0 - load all spectra from the file
	 * n - load first N spectra from the file
	 * for initial test in example n = 3
	 */
	int total_spectra = 0;

	/* Declare number of peaks you want to load from each spectrum
	 * 0 - load all peaks from the spectrum
	 * m - load first N peaks from the spectrum
	 * for initial test in example m = 5
	 */
	int num_peaks = 0;

	bool demo = false;
	QueryResults *q_res;
	
	if (std::getenv("DEMO")) {
		total_spectra = 3;
		num_peaks = 5;
		demo = true;
	}

	RawData * raw_data = load_raw_data(argv[1], total_spectra, num_peaks);
	//std::cerr << "raw_data=\n";
	//dump_raw_data(raw_data);
	
	std::vector<Spectrum> *queries = load_queries(argv[2]);
	q_res = init_query_results(queries->size());


	if (demo) {
		std::cerr << "queries=\n";
		for(auto &s: *queries) {
			dump_spectrum(&s);
			std::cerr << "\n";
		}
		std::cerr << "\n";
	}

	// Here's where the interesting part starts

	auto index_build_start = std::chrono::high_resolution_clock::now();
	Index * index = build_index(raw_data);
	auto index_build_end = std::chrono::high_resolution_clock::now();

	delete raw_data;
		
	if (demo) {
		std::cerr << "Index: ";
		dump_index(index);
	}

	auto reconstruct_start = std::chrono::high_resolution_clock::now();
	auto reconstructed_spectra = reconstruct_candidates(index, *queries, q_res);
	auto reconstruct_end = std::chrono::high_resolution_clock::now();	
	json_reconstruction(argv[3], *reconstructed_spectra);

	std::cerr << "Found " << reconstructed_spectra->size() << " candidates \n";
	std::cerr << "Building the index took " << (std::chrono::duration_cast<std::chrono::nanoseconds>(index_build_end - index_build_start).count()+0.0)/1e9 << " s\n";
	std::cerr << "Reconstruction took     " << (std::chrono::duration_cast<std::chrono::nanoseconds>(reconstruct_end - reconstruct_start).count()+0.0)/1e9 << " s\n";
}

