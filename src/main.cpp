// Minimal implementation focusing on user commands without STL containers
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
using namespace std;

struct User {
    string username, password, name, mail;
    int privilege;
    bool logged;
};

static const char *USER_FILE = "users.dat";
static const int MAX_USERS = 10000;
static User users[MAX_USERS];
static int user_count = 0;

static void load_users() {
    ifstream fin(USER_FILE);
    if (!fin) { user_count = 0; return; }
    int n; fin >> n;
    user_count = 0;
    for (int i = 0; i < n && i < MAX_USERS; ++i) {
        User u; fin >> u.username >> u.password >> u.name >> u.mail >> u.privilege;
        u.logged = false;
        if (fin) users[user_count++] = u;
    }
}

static void save_users() {
    ofstream fout(USER_FILE, ios::trunc);
    fout << user_count << '\n';
    for (int i = 0; i < user_count; ++i) {
        User &u = users[i];
        fout << u.username << ' ' << u.password << ' ' << u.name << ' ' << u.mail << ' ' << u.privilege << '\n';
    }
}

static int find_user(const string &uname) {
    for (int i = 0; i < user_count; ++i) if (users[i].username == uname) return i;
    return -1;
}

static bool is_first_user() { return user_count == 0; }

static int split_tokens(const string &line, string tokens[], int max_tokens) {
    int cnt = 0;
    string cur;
    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (isspace((unsigned char)c)) {
            if (!cur.empty()) {
                if (cnt < max_tokens) tokens[cnt++] = cur;
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty() && cnt < max_tokens) tokens[cnt++] = cur;
    return cnt;
}

struct KV { string key; string val; };

static int parse_kv(string tokens[], int tcnt, KV out[], int max_kv) {
    int m = 0;
    for (int i = 1; i < tcnt; ++i) {
        if (!tokens[i].empty() && tokens[i][0] == '-') {
            string key = tokens[i].substr(1);
            string val;
            if (i + 1 < tcnt && (tokens[i+1].empty() || tokens[i+1][0] != '-')) {
                val = tokens[i+1];
                ++i;
            }
            if (m < max_kv) { out[m].key = key; out[m].val = val; ++m; }
        }
    }
    return m;
}

static string get_kv(KV kv[], int m, const string &k) {
    for (int i = 0; i < m; ++i) if (kv[i].key == k) return kv[i].val; return "";
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    load_users();
    string line;
    while (true) {
        if (!getline(cin, line)) break;
        if (line.empty()) continue;
        string tokens[256];
        int tcnt = split_tokens(line, tokens, 256);
        if (tcnt <= 0) continue;
        string cmd = tokens[0];
        KV kv[128];
        int m = parse_kv(tokens, tcnt, kv, 128);

        if (cmd == "add_user") {
            string c = get_kv(kv, m, "c");
            string u = get_kv(kv, m, "u");
            string p = get_kv(kv, m, "p");
            string n = get_kv(kv, m, "n");
            string ml = get_kv(kv, m, "m");
            string g = get_kv(kv, m, "g");

            if (find_user(u) != -1) { cout << -1 << '\n'; continue; }
            if (is_first_user()) {
                if (user_count >= MAX_USERS) { cout << -1 << '\n'; continue; }
                User nu{u,p,n,ml,10,false};
                users[user_count++] = nu;
                save_users();
                cout << 0 << '\n';
                continue;
            }
            int ci = find_user(c); if (ci==-1 || !users[ci].logged) { cout << -1 << '\n'; continue; }
            if (g.empty()) { cout << -1 << '\n'; continue; }
            int gp = stoi(g);
            if (gp >= users[ci].privilege) { cout << -1 << '\n'; continue; }
            if (user_count >= MAX_USERS) { cout << -1 << '\n'; continue; }
            User nu{u,p,n,ml,gp,false};
            users[user_count++] = nu;
            save_users();
            cout << 0 << '\n';
        } else if (cmd == "login") {
            string u = get_kv(kv, m, "u");
            string p = get_kv(kv, m, "p");
            int i = find_user(u);
            if (i==-1 || users[i].password!=p) { cout << -1 << '\n'; continue; }
            if (users[i].logged) { cout << -1 << '\n'; continue; }
            users[i].logged = true;
            cout << 0 << '\n';
        } else if (cmd == "logout") {
            string u = get_kv(kv, m, "u");
            int i = find_user(u);
            if (i!=-1 && users[i].logged) { users[i].logged=false; cout << 0 << '\n'; }
            else cout << -1 << '\n';
        } else if (cmd == "query_profile") {
            string c = get_kv(kv, m, "c");
            string u = get_kv(kv, m, "u");
            int ci = find_user(c); int ui = find_user(u);
            if (ci==-1 || ui==-1 || !users[ci].logged) { cout << -1 << '\n'; continue; }
            if (!(users[ci].privilege > users[ui].privilege || c==u)) { cout << -1 << '\n'; continue; }
            cout << users[ui].username << ' ' << users[ui].name << ' ' << users[ui].mail << ' ' << users[ui].privilege << '\n';
        } else if (cmd == "modify_profile") {
            string c = get_kv(kv, m, "c");
            string u = get_kv(kv, m, "u");
            int ui = find_user(u);
            int ci = find_user(c);
            if (ci==-1 || ui==-1 || !users[ci].logged) { cout << -1 << '\n'; continue; }
            if (!(users[ci].privilege > users[ui].privilege || c==u)) { cout << -1 << '\n'; continue; }
            string np = get_kv(kv, m, "p"); if (!np.empty()) users[ui].password=np;
            string nn = get_kv(kv, m, "n"); if (!nn.empty()) users[ui].name=nn;
            string nm = get_kv(kv, m, "m"); if (!nm.empty()) users[ui].mail=nm;
            string ng = get_kv(kv, m, "g"); if (!ng.empty()) {
                int gpi = stoi(ng);
                if (gpi >= users[ci].privilege) { cout << -1 << '\n'; continue; }
                users[ui].privilege = gpi;
            }
            save_users();
            cout << users[ui].username << ' ' << users[ui].name << ' ' << users[ui].mail << ' ' << users[ui].privilege << '\n';
        } else if (cmd == "clean") {
            user_count = 0; save_users();
            cout << 0 << '\n';
        } else {
            if (cmd == "query_ticket") cout << 0 << '\n';
            else if (cmd == "query_transfer") cout << 0 << '\n';
            else if (cmd == "query_train") cout << -1 << '\n';
            else if (cmd == "add_train") cout << -1 << '\n';
            else if (cmd == "release_train") cout << -1 << '\n';
            else if (cmd == "delete_train") cout << -1 << '\n';
            else if (cmd == "buy_ticket") cout << -1 << '\n';
            else if (cmd == "query_order") cout << -1 << '\n';
            else if (cmd == "refund_ticket") cout << -1 << '\n';
            else cout << -1 << '\n';
        }
    }
    return 0;
}
