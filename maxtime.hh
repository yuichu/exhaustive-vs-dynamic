///////////////////////////////////////////////////////////////////////////////
// maxtime.hh
//
// Compute the set of rides that maximizes the time spent at rides, within a given budget
// with the dynamic method or exhaustive search.
//
///////////////////////////////////////////////////////////////////////////////


#pragma once


#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <vector>


// One ride item available for purchase.
class RideItem
{
	//
	public:

		//
		RideItem
		(
			const std::string& description,
			size_t cost_dollars,
			double time_minutes
		)
			:
			_description(description),
			_cost_dollars(cost_dollars),
			_time_minutes(time_minutes)
		{
			assert(!description.empty());
			assert(cost_dollars > 0);
		}

		//
		const std::string& description() const { return _description; }
		int cost() const { return _cost_dollars; }
		double defense() const { return _time_minutes; }

	//
	private:

		// Human-readable description of the ride, e.g. "new enchanted world". Must be non-empty.
		std::string _description;

		// Ride cost, in units of dollars; Must be positive
		int _cost_dollars;

		// Ride time in minutes; most be non-negative.
		double _time_minutes;
};


// Alias for a vector of shared pointers to RideItem objects.
typedef std::vector<std::shared_ptr<RideItem>> RideVector;


// Load all the valid ride items from the CSV database
// ride items that are missing fields, or have invalid values, are skipped.
// Returns nullptr on I/O error.
std::unique_ptr<RideVector> load_ride_database(const std::string& path)
{
	std::unique_ptr<RideVector> failure(nullptr);

	std::ifstream f(path);
	if (!f)
	{
		std::cout << "Failed to load ride database; Cannot open file: " << path << std::endl;
		return failure;
	}

	std::unique_ptr<RideVector> result(new RideVector);

	size_t line_number = 0;
	for (std::string line; std::getline(f, line); )
	{
		line_number++;

		// First line is a header row
		if ( line_number == 1 )
		{
			continue;
		}

		std::vector<std::string> fields;
		std::stringstream ss(line);

		for (std::string field; std::getline(ss, field, '^'); )
		{
			fields.push_back(field);
		}

		if (fields.size() != 3)
		{
			std::cout
				<< "Failed to load ride database: Invalid field count at line " << line_number << "; Want 3 but got " << fields.size() << std::endl
				<< "Line: " << line << std::endl
				;
			return failure;
		}

		std::string
			descr_field = fields[0],
			cost_dollars_field = fields[1],
			time_minutes_field = fields[2]
			;

		auto parse_dbl = [](const std::string& field, double& output)
		{
			std::stringstream ss(field);
			if ( ! ss )
			{
				return false;
			}

			ss >> output;

			return true;
		};

		std::string description(descr_field);
		double cost_dollars, time_minutes;
		if (
			parse_dbl(cost_dollars_field, cost_dollars)
			&& parse_dbl(time_minutes_field, time_minutes)
		)
		{
			result->push_back(
				std::shared_ptr<RideItem>(
					new RideItem(
						description,
						cost_dollars,
						time_minutes
					)
				)
			);
		}
	}

	f.close();

	return result;
}


// Convenience function to compute the total cost and time in a RideVector.
// Provide the RideVector as the first argument
// The next two arguments will return the cost and time back to the caller.
void sum_ride_vector
(
	const RideVector& rides,
	int& total_cost,
	double& total_time
)
{
	total_cost = total_time = 0;
	for (auto& ride : rides)
	{
		total_cost += ride->cost();
		total_time += ride->defense();
	}
}


// Convenience function to print out each RideItem in a RideVector,
// followed by the total kilocalories and protein in it.
void print_ride_vector(const RideVector& rides)
{
	std::cout << "*** ride Vector ***" << std::endl;

	if ( rides.size() == 0 )
	{
		std::cout << "[empty ride list]" << std::endl;
	}
	else
	{
		for (auto& ride : rides)
		{
			std::cout
				<< "Ye olde " << ride->description()
				<< " ==> "
				<< "Cost of " << ride->cost() << " dollars"
				<< "; time points = " << ride->defense()
				<< std::endl
				;
		}

		int total_cost;
		double total_time;
		sum_ride_vector(rides, total_cost, total_time);
		std::cout
			<< "> Grand total cost: " << total_cost << " dollars" << std::endl
			<< "> Grand total defense: " << total_time
			<< std::endl
			;
	}
}


// Convenience function to print out a 2D cache, composed of a std::vector<std::vector<double>>
// For sanity, will refuse to print a cache that is too large.
// Hint: When running this program, you can redirect stdout to a file,
//	which may be easier to view and inspect than a terminal
void print_2d_cache(const std::vector<std::vector<double>>& cache)
{
	std::cout << "*** 2D Cache ***" << std::endl;

	if ( cache.size() == 0 )
	{
		std::cout << "[empty]" << std::endl;
	}
	else if ( cache.size() > 250 || cache[1].size() > 250 )
	{
		std::cout << "[too large]" << std::endl;
	}
	else
	{
		for ( const std::vector<double>& row : cache)
		{
			for ( double value : row )
			{
				std::cout << std::setw(5) << value;
			}
			std::cout << std::endl;
		}
	}
}

// Filter the vector source, i.e. create and return a new RideVector containing the subset of 
// the ride items in source that match given criteria.
// This is intended to:
//	1) filter out ride with zero or negative time that are irrelevant to our optimization
//	2) limit the size of inputs to the exhaustive search algorithm since it will probably be slow.
//
// Each ride item that is included must have at minimum min_time and at most max_time.
//	(i.e., each included ride item's time must be between min_time and max_time (inclusive).
//
// In addition, the the vector includes only the first total_size ride items that match these criteria.
std::unique_ptr<RideVector> filter_ride_vector
(
	const RideVector& source,
	double min_time,
	double max_time,
	int total_size
)
{
std::unique_ptr<RideVector> sortedVector(new RideVector);
for(size_t i = 0; i < source.size(); i++)
{
	if((*source[i]).defense() > 0 && ((*source[i]).defense() >= min_time && (*source[i]).defense() <= max_time) && (*sortedVector).size() < total_size)
	{
		(*sortedVector).push_back(source[i]);
	}
}
	return sortedVector;
}

// Compute the optimal set of ride items with a dynamic algorithm.
// Specifically, among the ride items that fit within a total_cost budget,
// choose the selection of rides whose time is greatest.
// Repeat until no more ride items can be chosen, either because we've run out of ride items,
// or run out of dollars.
std::unique_ptr<RideVector> dynamic_max_time
(
	const RideVector& rides,
	int total_cost
)
{
   // TODO: implement this function, then delete the return statement below
	return nullptr;
}

std::vector<std::vector<RideItem>> getDefenseSubsets(std::vector<RideItem> source)
{
    std::vector<std::vector<RideItem>> subset, subTemp;

    std::vector<RideItem> temp;
    subset.push_back(temp);

    for(int i = 0; i < source.size(); i++)
    {
        subTemp = subset;
        for(int j = 0; j < subTemp.size(); j++){subTemp[j].push_back(source[i]);}
        for(int k = 0; k < subTemp.size(); k++){subset.push_back(subTemp[k]);}
    }
    return subset;
}

// Compute the optimal set of ride items with a exhaustive search algorithm.
// Specifically, among all subsets of ride items,
// return the subset whose dollars cost fits within the total_cost budget,
// and whose total time is greatest.
// To avoid overflow, the size of the ride items vector must be less than 64.
std::unique_ptr<RideVector> exhaustive_max_time
(
	const RideVector& rides,
	double total_cost
)
{
	std::unique_ptr<RideVector> best1(new RideVector);
	size_t n = rides.size();
	int candidateTotalCost = 0;
	double candidateTotalTime = 0;
	int bestTotalCost = 0;
	double bestTotalTime = 0;

	//ride items vector must be less than 64 to avoid overflow
	if (rides.size() >= 64)
	{
		exit(1);	// if ride size is greater than 64, exit program
	}

	for (uint64_t bits = 0; bits < pow(2, n); bits++)
	{
		std::unique_ptr<RideVector> candidate1(new RideVector);

		for (uint64_t j = 0; j < n; j++)
		{
			if (((bits >> j) & 1) == 1)
			{
				(*candidate1).push_back(rides[j]);
			}
		}
		
		// calculate total cost and total time of candidate and best
		sum_ride_vector(*candidate1, candidateTotalCost, candidateTotalTime);
		sum_ride_vector(*best1, bestTotalCost, bestTotalTime);

		// move candidate to best if within budget and has greater total time than current best
		if (candidateTotalCost <= total_cost)
		{
			if ((*best1).empty() || candidateTotalTime > bestTotalTime)
			{
				*best1 = *candidate1;
			}
		}
	}
	return best1;
}









