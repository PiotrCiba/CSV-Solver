// CSV_Solver.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#define no_init_all
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <istream>
#include <string>
#include <vector>
#include <cctype>
#include <iomanip>

#ifndef NOMINMAX
#define NOMINMAX
#endif

using namespace std;

enum class CSVState {
	UnquotedField,
	QuotedField,
	QuotedQuote
};

vector<string> readCSVRow(const string& row) {
	CSVState state = CSVState::UnquotedField;
	std::vector<std::string> fields{ "" };
	size_t i = 0; // index of the current field
	for (char c : row) {
		switch (state) {
		case CSVState::UnquotedField:
			switch (c) {
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
			switch (c) {
			case '"': state = CSVState::QuotedQuote;
				break;
			default:  fields[i].push_back(c);
				break;
			}
			break;
		case CSVState::QuotedQuote:
			switch (c) {
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

// Read CSV file, Excel dialect. Accept "quoted fields ""with quotes"""
vector<vector<string>> readCSV(istream& in) {
	std::vector<std::vector<std::string>> table;
	std::string row;
	while (!in.eof()) {
		std::getline(in, row);
		if (in.bad() || in.fail()) {
			break;
		}
		auto fields = readCSVRow(row);
		table.push_back(fields);
	}
	return table;
}

/*
//Read the table of voltage and note the times when it crosses the zero.
//If a zero happens to be noted, write one entry, if not, write the neighouring ones.
vector<vector<double>> zeroTable(vector<vector<double>> input) {
	vector<vector<double>> output;
	double last, now;
	for (size_t i = 1;i < input.size();i++) {
		now = input[i][1];
		vector<double> tmp;
		if (now == 0) {
			tmp.push_back(input[i][0]);
			output.push_back(tmp);
		}
		if (i>1) {
			if ((now > 0 && last < 0) || (now < 0 && last > 0)) {
				tmp.push_back(last);
				tmp.push_back(now);
				//tmp.push_back(input[i-1][0]);
				//tmp.push_back(input[i][0]);
				output.push_back(tmp);

			}
		}
		last = now;
	}
	return output;
}
*/



int main()
{
	char ofilename[MAX_PATH];
	char sfilename[MAX_PATH] = "output";
	char sfext[MAX_PATH] = ".csv\0";

	OPENFILENAMEA ofn, sfn;
	ZeroMemory(&ofilename, sizeof(ofilename));
	ZeroMemory(&sfilename, sizeof(sfilename));
	ZeroMemory(&ofn, sizeof(ofn));
	ZeroMemory(&sfn, sizeof(sfn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	ofn.lpstrFilter = ".scv Files\0*.csv\0Any File\0*.*\0";
	ofn.lpstrFile = ofilename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Select a File to Open";
	ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

	sfn.lStructSize = sizeof(sfn);
	sfn.hwndOwner = NULL;  // If you have a window to center over, put its HANDLE here
	sfn.lpstrFilter = ofn.lpstrDefExt = sfext;
	sfn.lpstrFile = sfilename;
	sfn.nMaxFile = MAX_PATH;
	sfn.lpstrTitle = "Select a File to Save";
	sfn.Flags = OFN_DONTADDTORECENT | OFN_CREATEPROMPT | OFN_OVERWRITEPROMPT;


	string choice = "";
	vector<double> time;
	vector<double> v_in;
	vector<double> v_out;

	//setup for FIR filter, with defaults
	const int N = 4;					//order
	const float samplerate = 1000;	//Hz
	const float cutoff = 20;			//Hz
	//end FIR setup

	while (choice != "x") {
		system("CLS");
		choice = "";
		cout << "=========================================" << endl;
		cout << "=\t\t CSV Solver\t\t=" << endl;
		cout << "=\t\tby PiotrCiba\t\t=" << endl;
		cout << "=\t\tberndporr/iir1\t\t=" << endl;
		cout << "=\t\tMathGL & FLTH\t\t=" <<endl;
		cout << "=========================================" << endl;
		string msg = (!v_in.empty()) ? "Wczytany" : "Pusty";
		cout << "Input file: " << msg << endl;
		if (!v_in.empty()) {
			cout << "----------------------------------" << endl;
			cout << "Nazwa: " << ofilename << endl;
			cout << "Pozycje " << v_in.size() << endl;
			cout << "----------------------------------" << endl;
		}
		cout << "Wybierz akcje do wykonania:" << endl;
		cout << "\t(o)Wczytaj plik .CSV" << endl;
		cout << "\t(s)Zapisz dane do pliku .CSV" << endl;
		cout << "\t(a)Automatyczne" << endl;
		cout << "\t(c)Wlasny filtr(TODO)" << endl;
		cout << "\t(g)Wykres sygnalu wyjscia(TODO)" << endl;
		cout << "\t(x)Zamknij program" << endl;
		cout << "=========================================" << endl;
		cin >> choice;
		cout << "=========================================" << endl;
		if (choice == "o") {
			if (GetOpenFileNameA(&ofn))
			{
				cout << "Wybrales plik: \"" << ofilename << "\"\n";
				ifstream input(ofilename);
				if (!input.is_open()) return 1;
				cout << "Plik otwarty \nWczytywanie:\t";
				vector<vector<string>> tmp = readCSV(input);
				tmp.erase(tmp.begin());
				int progress = 0;
				double offset = abs(stod(tmp[0][0]));
				for (vector<string> i : tmp) {
					time.push_back(stod(i[0]) * 1000 + offset);
					v_in.push_back(stod(i[1]));
					progress++;
					cout << ((progress % 1000 == 0) ? "." : "");
				}
				cout << endl << "Plik wczytany.";
				input.close();
			}
			else
			{
				// All this stuff below is to tell you exactly how you messed up above. 
				// Once you've got that fixed, you can often (not always!) reduce it to a 'user cancelled' assumption.
				switch (CommDlgExtendedError())
				{
				case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";   break;
				case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";  break;
				case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";  break;
				case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";  break;
				case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";  break;
				case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";  break;
				case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
				case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
				case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";     break;
				case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";          break;
				case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";      break;
				case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";      break;
				case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
				case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
				case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
				default: std::cout << "You cancelled.\n";
				}
			}
		}
		if (choice == "s") {
			if (GetSaveFileNameA(&sfn))
			{
				if (!v_out.empty()) {
					for (size_t i = 0;i < v_out.size() || i < v_in.size();i++) {
						v_in[i] = v_out[i];
					}
				}
				cout << "Zapisujesz do pliku \"" << sfilename << "\"\n";
				ofstream output(sfilename);
				if (!output.is_open()) return 1;
				output << "t[ms],U[Volts]" << endl << fixed << setprecision(16);
				for (size_t i = 0;i < v_in.size();i++) {
					output << time[i] << "," << v_in[i] << endl;
				}
				cout << "Plik zapisany.\n";
			}
			else
			{
				// All this stuff below is to tell you exactly how you messed up above. 
				// Once you've got that fixed, you can often (not always!) reduce it to a 'user cancelled' assumption.
				switch (CommDlgExtendedError())
				{
				case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n";   break;
				case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n";  break;
				case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n";  break;
				case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n";  break;
				case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n";  break;
				case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n";  break;
				case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
				case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n";  break;
				case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n";     break;
				case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n";          break;
				case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n";      break;
				case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n";      break;
				case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n";  break;
				case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
				case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
				default: std::cout << "You cancelled.\n";
				}
			}
		}
		/*
		if (choice == "a" && !v_in.empty()) {
			cout << "domyślnie:\tFiltr IIR Butterswortha, dolnoprzepustowy" << endl;
			cout << "Rzad:\t" << N << "\nProbkowanie:\t" << samplerate << "\nPasmo odciecia:\t" << cutoff << endl << endl;
			Iir::Butterworth::LowPass<N> f;
			f.setup(samplerate, cutoff);
			double tmp;
			cout << "Postep:\t";

			for (size_t i = 0;i < v_in.size();i++) {
				tmp = f.filter(v_in[i]);
				v_out.push_back(tmp);
				cout << ((i % 1000 == 0) ? "." : "");
			}
			cout << endl << "Wykonano";
		}
		else if (v_in.empty()) {
			cout << "Auto Err: Plik nie wczytany";
		}
		*/
		if (choice == "g" && !v_in.empty()) {

		}
	}

	return 0;
}


