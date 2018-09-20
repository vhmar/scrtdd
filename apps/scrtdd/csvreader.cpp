/***************************************************************************
 *   Copyright (C) by ETHZ/SED                                             *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Developed by Luca Scarabello <luca.scarabello@sed.ethz.ch>            *
 ***************************************************************************/

#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <istream>
#include <fstream>
#include <iostream>

#include "csvreader.h"

using namespace std;

namespace CSV {

enum class CSVState {
    UnquotedField,
    QuotedField,
    QuotedQuote
};

vector<string> readRow(const string &row)
{
	CSVState state = CSVState::UnquotedField;
	vector<string> fields {""};
	size_t i = 0; // index of the current field
	for (char c : row)
	{
		switch (state)
		{
			case CSVState::UnquotedField:
				switch (c)
				{
					case ',': // end of field
							  fields.push_back(""); i++;
							  break;
					case '"': state = CSVState::QuotedField;
							  break;
					default:  fields[i].push_back(c);
							  break;
				}
				break;
			case CSVState::QuotedField:
				switch (c)
				{
					case '"': state = CSVState::QuotedQuote;
							  break;
					default:  fields[i].push_back(c);
							  break;
				}
				break;
			case CSVState::QuotedQuote:
				switch (c)
				{
					case ',': // , after closing quote
							  fields.push_back(""); i++;
							  state = CSVState::UnquotedField;
							  break;
					case '"': // "" -> "
							  fields[i].push_back('"');
							  state = CSVState::QuotedField;
							  break;
					default:  // end of quote
							  state = CSVState::UnquotedField;
							  break;
				}
				break;
		}
	}
	return fields;
}

/// Read CSV file, Excel dialect. Accept "quoted fields ""with quotes"""
vector<vector<string>> read(istream &in)
{
	vector<vector<string>> table;
	string row;
	while (!in.eof())
	{
		getline(in, row);
		if (in.bad() || in.fail()) {
			break;
		}
		auto fields = readRow(row);
		table.push_back(fields);
	}
	return table;
}

vector<vector<string>> read(const string &filename)
{
	ifstream csvfile(filename);
	return read(csvfile);
}

vector< map<string,string> > format(const vector<string>& header,
                                    const vector<vector<string>>::const_iterator& begin,
                                    const vector<vector<string>>::const_iterator& end)
{
	vector< map<string,string> > rows;
	for (auto it = begin; it != end; it++)
	{
		const vector<string> columns = *it;
		map<string,string> row;
		for (size_t i = 0; i < header.size(); ++i)
		{
			row[ header[i] ] = columns[i];
		}
		rows.push_back(row);
	}
	return rows;
}

vector< map<string,string> > readWithHeader(istream &in)
{
	vector<vector<string>> rows = read(in);
	return format(rows[0], rows.begin()+1, rows.end());
}

vector< map<string,string> > readWithHeader(istream &in,
                                            const vector<string>& header)
{
	vector<vector<string>> rows = read(in);
	return format(header, rows.begin(), rows.end());
}

vector< map<string,string> > readWithHeader(const string &filename)
{
	ifstream csvfile(filename);
	return readWithHeader(csvfile);
}

vector< map<string,string> > readWithHeader(const string &filename,
                                            const vector<string>& header)
{
	ifstream csvfile(filename);
	return readWithHeader(csvfile, header);
}

} // CSV