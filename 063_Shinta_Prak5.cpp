#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstdlib>

using namespace std;


const int HASH_SIZE = 20;

struct Movie;
struct Genre;

struct MovieEdge {
    Movie* target;
    MovieEdge* next;
};

struct GenreEdge {
    Genre* genre;
    GenreEdge* next;
};

struct GenreMovieNode {
    Movie* movie;
    GenreMovieNode* next;
};

struct Genre {
    string nama;
    GenreMovieNode* movies;
    Genre* next;
};

struct Movie {
    string nama;
    string studio;

    int jumlahEpisode;
    int jumlahSeason;

    int totalUserRating;
    float rating;

    Movie* left;
    Movie* right;

    MovieEdge* relatedMovies;
    GenreEdge* genres;
};

Movie* root = NULL;
Genre* genreTable[HASH_SIZE];

bool isAdmin = false;


int hashGenre(string s) {
    int sum = 0;
    for(int i = 0; i < (int)s.length(); i++) {
        sum += s[i];
    }
    return sum % HASH_SIZE;
}

Genre* findGenre(string nama) {
    int idx = hashGenre(nama);
    Genre* curr = genreTable[idx];
    while(curr) {
        if(curr->nama == nama) return curr;
        curr = curr->next;
    }
    return NULL;
}

void insertGenre(string nama) {
    int idx = hashGenre(nama);
    Genre* g = new Genre;
    g->nama = nama;
    g->movies = NULL;
    g->next = genreTable[idx];
    genreTable[idx] = g;
}


Movie* createMovie(string nama, string studio, int ep, int season) {
    Movie* m = new Movie;
    m->nama = nama;
    m->studio = studio;
    m->jumlahEpisode = ep;
    m->jumlahSeason = season;
    m->rating = 0;
    m->totalUserRating = 0;
    m->left = NULL;
    m->right = NULL;
    m->relatedMovies = NULL;
    m->genres = NULL;
    return m;
}

Movie* searchMovie(Movie* root, string nama) {
    if(root == NULL) return NULL;
    if(root->nama == nama) return root;
    if(nama < root->nama)
        return searchMovie(root->left, nama);
    return searchMovie(root->right, nama);
}

Movie* insertMovie(Movie* root, Movie* movie) {
    if(root == NULL) return movie;
    if(movie->nama < root->nama)
        root->left = insertMovie(root->left, movie);
    else if(movie->nama > root->nama)
        root->right = insertMovie(root->right, movie);
    return root;
}


bool isConnected(Movie* a, Movie* b) {
    MovieEdge* curr = a->relatedMovies;
    while(curr) {
        if(curr->target == b) return true;
        curr = curr->next;
    }
    return false;
}

void connectMovie(Movie* a, Movie* b) {
    if(a == NULL || b == NULL) return;
    if(isConnected(a, b)) return;

    MovieEdge* edge1 = new MovieEdge;
    edge1->target = b;
    edge1->next = a->relatedMovies;
    a->relatedMovies = edge1;

    MovieEdge* edge2 = new MovieEdge;
    edge2->target = a;
    edge2->next = b->relatedMovies;
    b->relatedMovies = edge2;
}

void connectGenreMovie(Genre* g, Movie* m) {
    GenreMovieNode* gm = new GenreMovieNode;
    gm->movie = m;
    gm->next = g->movies;
    g->movies = gm;

    GenreEdge* ge = new GenreEdge;
    ge->genre = g;
    ge->next = m->genres;
    m->genres = ge;
}


void saveRecursive(Movie* node, ofstream &file) {
    if (!node) return;
    saveRecursive(node->left, file);
    file << node->nama << "," << node->studio << "," << node->rating << "," 
         << node->totalUserRating << "," << node->jumlahEpisode << "," << node->jumlahSeason << ",";
    GenreEdge* ge = node->genres;
    while(ge) { file << ge->genre->nama << (ge->next ? "|" : ""); ge = ge->next; }
    file << ",";
    MovieEdge* me = node->relatedMovies;
    while(me) { file << me->target->nama << (me->next ? "|" : ""); me = me->next; }
    file << "\n";
    saveRecursive(node->right, file);
}

void exportToCSV() {
    ofstream file("data_wordboxd.csv");
    if (file.is_open()) {
        saveRecursive(root, file);
        file.close();
    }
}

void importFromCSV() {
    ifstream file("data_wordboxd.csv");
    if (!file.is_open()) return;
    string line;
    while (getline(file, line)) {
        if(line.empty()) continue;
        stringstream ss(line);
        string n, s, r, tr, ep, se, gs, rs;
        getline(ss, n, ','); getline(ss, s, ','); getline(ss, r, ',');
        getline(ss, tr, ','); getline(ss, ep, ','); getline(ss, se, ',');
        getline(ss, gs, ','); getline(ss, rs, ',');
        Movie* m = createMovie(n, s, atoi(ep.c_str()), atoi(se.c_str()));
        m->rating = (float)atof(r.c_str()); 
        m->totalUserRating = atoi(tr.c_str());
        root = insertMovie(root, m);
        stringstream ssG(gs); string gName;
        while (getline(ssG, gName, '|')) {
            Genre* g = findGenre(gName);
            if (g) connectGenreMovie(g, m);
        }
    }
    file.clear(); file.seekg(0);
    while (getline(file, line)) {
        if(line.empty()) continue;
        stringstream ss(line); string n, d1, d2, d3, d4, d5, d6, rs;
        getline(ss, n, ','); getline(ss, d1, ','); getline(ss, d2, ',');
        getline(ss, d3, ','); getline(ss, d4, ','); getline(ss, d5, ',');
        getline(ss, d6, ','); getline(ss, rs, ',');
        Movie* cur = searchMovie(root, n);
        stringstream ssR(rs); string rName;
        while (getline(ssR, rName, '|')) {
            Movie* target = searchMovie(root, rName);
            if (target) connectMovie(cur, target);
        }
    }
    file.close();
}


void printGenres(Movie* m) {
    GenreEdge* curr = m->genres;
    bool first = true;
    while(curr) {
        if(!first) cout << ", ";
        cout << curr->genre->nama;
        first = false;
        curr = curr->next;
    }
    cout << endl;
}

void printRelated(Movie* m) {
    MovieEdge* curr = m->relatedMovies;
    if(curr == NULL) {
        cout << "-" << endl;
        return;
    }
    while(curr) {
        cout << "- " << curr->target->nama << endl;
        curr = curr->next;
    }
}

void displayMovie(Movie* m) {
    cout << "\n---- " << m->nama << " ----\n";
    cout << "Genre: ";
    printGenres(m);
    if(m->jumlahEpisode == 0) cout << "Jumlah ep: -\n";
    else cout << "Jumlah ep: " << m->jumlahEpisode << endl;
    if(m->jumlahSeason == 0) cout << "Jumlah season: -\n";
    else cout << "Jumlah season: " << m->jumlahSeason << endl;
    cout << "Film terkait:\n";
    printRelated(m);
    cout << "Studio: " << m->studio << endl;
    cout << fixed << setprecision(1);
    cout << "Rating: " << m->rating << endl;
}

void rateMovie(Movie* m) {
    float input;
    cout << "\n==== Rate " << m->nama << " ====\n";
    cout << "Rating (1-10): ";
    cin >> input;
    if(input < 1 || input > 10) {
        cout << "Rating tidak valid\n";
        return;
    }
    float total = (m->rating * m->totalUserRating) + input;
    m->totalUserRating++;
    m->rating = total / m->totalUserRating;
    cout << "Rating terbaru: " << fixed << setprecision(1) << m->rating << endl;
    exportToCSV(); 
}


Movie* movieList[1000];
int movieCount = 0;

void resetList() { movieCount = 0; }

void collectMovies(Movie* root, bool seriesOnly) {
    if(root == NULL) return;
    collectMovies(root->left, seriesOnly);
    bool isSeries = root->jumlahEpisode > 0 || root->jumlahSeason > 0;
    if(seriesOnly && isSeries) movieList[movieCount++] = root;
    if(!seriesOnly && !isSeries) movieList[movieCount++] = root;
    collectMovies(root->right, seriesOnly);
}

void menuAll(bool seriesOnly) {
    resetList();
    collectMovies(root, seriesOnly);
    if(seriesOnly) cout << "\n==== Semua Series ====\n";
    else cout << "\n==== Semua Film ====\n";
    for(int i = 0; i < movieCount; i++) {
        cout << i + 1 << ". " << movieList[i]->nama << " | " 
             << fixed << setprecision(1) << movieList[i]->rating << endl;
    }
    char pilih;
    cout << "\n[R]ate / [C]lose : ";
    cin >> pilih;
    if(pilih == 'R' || pilih == 'r') {
        int idx;
        cout << "Pilih film: ";
        cin >> idx;
        if(idx >= 1 && idx <= movieCount) rateMovie(movieList[idx - 1]);
    }
}


void searchMenu() {
    cin.ignore();
    string nama;
    cout << "\n==== Search Film / Series ====\n";
    cout << "Nama: ";
    getline(cin, nama);
    Movie* m = searchMovie(root, nama);
    if(m == NULL) {
        cout << "Film tidak ditemukan\n";
        return;
    }
    displayMovie(m);
    char pilih;
    cout << "\n[R]ate / [C]lose : ";
    cin >> pilih;
    if(pilih == 'R' || pilih == 'r') rateMovie(m);
}


string genreList[] = {"War","Mystery","Horror","Action","Drama","Documentary","Musical","Comedy","Romance","Slice of life"};
int genreCount = 10;

void genreMenu() {
    cout << "\n==== Pilih Genre ====\n";
    for(int i = 0; i < genreCount; i++) cout << i + 1 << ". " << genreList[i] << endl;
    int pilih;
    cout << "Pilih: ";
    cin >> pilih;
    if(pilih < 1 || pilih > genreCount) return;
    string selected = genreList[pilih - 1];
    Genre* g = findGenre(selected);
    if(g == NULL || g->movies == NULL) {
        cout << "Tidak ada film\n";
        return;
    }
    Movie* temp[100];
    int cnt = 0;
    GenreMovieNode* curr = g->movies;
    while(curr) { temp[cnt++] = curr->movie; curr = curr->next; }
    int idx = 0;
    char cmd;
    while(true) {
        cout << "\n==== Genre " << selected << " ====\n";
        displayMovie(temp[idx]);
        cout << "\n[N]ext [P]revious [R]ate [C]lose : ";
        cin >> cmd;
        if(cmd == 'N' || cmd == 'n') { idx++; if(idx >= cnt) idx = 0; }
        else if(cmd == 'P' || cmd == 'p') { idx--; if(idx < 0) idx = cnt - 1; }
        else if(cmd == 'R' || cmd == 'r') rateMovie(temp[idx]);
        else if(cmd == 'C' || cmd == 'c') break;
    }
}


void tambahFilm() {
    cin.ignore();
    string nama, studio, genreInput, relatedInput;
    int ep, season;
    cout << "\n==== Tambah Film / Series ====\n";
    cout << "Nama: "; getline(cin, nama);
    if(searchMovie(root, nama)) {
        cout << nama << " sudah terdaftar di database\n";
        return;
    }
    cout << "Genre (pisah koma): "; getline(cin, genreInput);
    cout << "Jumlah Episode: "; cin >> ep;
    cout << "Jumlah Season: "; cin >> season;
    cin.ignore();
    cout << "Film terkait (pisah koma): "; getline(cin, relatedInput);
    cout << "Studio: "; getline(cin, studio);

    Movie* newMovie = createMovie(nama, studio, ep, season);
    bool validGenre = false;
    string temp = "";
    for(int i = 0; i <= (int)genreInput.length(); i++) {
        if(i == (int)genreInput.length() || genreInput[i] == ',') {
            while(!temp.empty() && temp[0] == ' ') temp.erase(0,1);
            Genre* g = findGenre(temp);
            if(g == NULL) cout << "Genre " << temp << " tidak terdaftar\n";
            else { validGenre = true; connectGenreMovie(g, newMovie); }
            temp = "";
        } else temp += genreInput[i];
    }
    if(!validGenre) { cout << "Semua genre invalid\n"; delete newMovie; return; }
    root = insertMovie(root, newMovie);
    temp = "";
    for(int i = 0; i <= (int)relatedInput.length(); i++) {
        if(i == (int)relatedInput.length() || relatedInput[i] == ',') {
            while(!temp.empty() && temp[0] == ' ') temp.erase(0,1);
            Movie* related = searchMovie(root, temp);
            if(related == NULL) cout << "Film " << temp << " tidak dapat ditemukan\n";
            else connectMovie(newMovie, related);
            temp = "";
        } else temp += relatedInput[i];
    }
    cout << "Film berhasil ditambahkan!\n";
    exportToCSV();
}


void initGenre() {
    for(int i = 0; i < HASH_SIZE; i++) genreTable[i] = NULL;
    for(int i = 0; i < 11; i++) insertGenre(genreList[i]);
}

int main() {
    initGenre();
    importFromCSV();


    while(true) {
        cout << "\n==== Wordboxd ====\n";
        cout << "1. Genre\n2. Search Film / Series\n3. Semua Film\n4. Semua Series\n";
        if(isAdmin) cout << "5. Tambah Film / Series\n";
        cout << "Pilihan: ";
        int pilih;
        if (!(cin >> pilih)) {
            cin.clear(); cin.ignore(1000, '\n');
            cout << "Input tidak valid!\n"; continue; 
        }
        if(pilih == 0) {
            string pass; cout << "Passphrase: "; cin >> pass;
            if(pass == "akuadmin727") { isAdmin = true; cout << "Admin mode aktif!\n"; }
            else cout << "Passphrase salah\n";
        }
        else if(pilih == 1) genreMenu();
        else if(pilih == 2) searchMenu();
        else if(pilih == 3) menuAll(false);
        else if(pilih == 4) menuAll(true);
        else if(pilih == 5 && isAdmin) tambahFilm();
        else cout << "Menu tidak tersedia.\n";
    }
    return 0;
}
