#pragma once

#include <vector>
#include "borrow_record.h"

extern std::vector<BorrowRecord> g_borrows;

void ReloadBorrows();
void SaveBorrows();
void borrowUIMain();
void borrowMyRecordsUIMain(int readerId);
