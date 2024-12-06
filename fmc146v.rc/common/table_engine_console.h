#ifndef TABLE_ENGINE_CONSOLE_H_
#define TABLE_ENGINE_CONSOLE_H_

#include "table_engine.h"
#if defined(__linux__)
#include "term_table.h"
#endif

#include <mutex>

class TableEngineConsole : public TableEngine
{
public:
    TableEngineConsole();
    ~TableEngineConsole();

public:
    int CreateTable(const char *pColumnName[], unsigned nCount);
    int CreateTable(const std::vector<std::string>& columns);
    int AddRowTable();
    int SetValueTable(unsigned nRow, unsigned nColumn, const char *fmt, ...);
    //int SetValueTable(unsigned nRow, unsigned nColumn, const char *pVal);
    //int SetValueTable(unsigned nRow, unsigned nColumn, signed nVal, const char* format="%d");
    //int SetValueTable(unsigned nRow, unsigned nColumn, unsigned nVal, const char *format="%u");
    //int SetValueTable(unsigned nRow, unsigned nColumn, float dVal, const char *format="%f");
    void ClearTable();
    void SaveTable();
    void UpdateTable();
    void GetConsolePos(int& X, int& Y);
    void SetConsolePos(int  X, int  Y);

private:
#if defined(__linux__)
    int m_C, m_R;
    int WC, HC, WCL;
    int WS, HS, XC, YC;
    int YLAST;
    int XLAST;
    std::vector<struct row_t> rows;
    std::mutex _lock;

    int get_screen(int *W, int *H);
    int get_pos(int *X, int *Y);
    void cell_draw(int x, int y, int w, int h);
    void cell_draw_fix();
    cell_t& get_cell(int row, int col);
#else
#endif
};

using console_table_t = std::shared_ptr<TableEngineConsole>;
inline console_table_t create_table(const std::vector<std::string>& col)
{
    console_table_t t = std::make_shared<TableEngineConsole>();
    if(t) t->CreateTable(col);
    return t;
}

#endif // TABLE_ENGINE_CONSOLE_H_
