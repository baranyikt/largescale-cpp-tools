#include "expires.h"

__EXPIRES__("20200202");
constexpr auto test1_1a = CodeExpiresFeature::details::DATE_FORMAT_ARG_VALID;
constexpr auto test1_1b = CodeExpiresFeature::details::DATE_FORMAT_COMPILER_VALID;
constexpr auto test1_1c = CodeExpiresFeature::details::CheckArg("20200202");
constexpr auto test1_2a = CodeExpiresFeature::details::COMPILE_DATE;
constexpr auto test1_2b = CodeExpiresFeature::details::ArgDate("20200202");
constexpr auto test1_3a = CodeExpiresFeature::ExpiresConditionCheck("20200202");

__EXPIRES__("20000101");
constexpr auto test2_1a = CodeExpiresFeature::details::DATE_FORMAT_ARG_VALID;
constexpr auto test2_1b = CodeExpiresFeature::details::DATE_FORMAT_COMPILER_VALID;
constexpr auto test2_1c = CodeExpiresFeature::details::CheckArg("20000101");
constexpr auto test2_2a = CodeExpiresFeature::details::COMPILE_DATE;
constexpr auto test2_2b = CodeExpiresFeature::details::ArgDate("20000101");
constexpr auto test2_3a = CodeExpiresFeature::ExpiresConditionCheck("20000101");

__EXPIRES__("hello");
constexpr auto test3_1a = CodeExpiresFeature::details::DATE_FORMAT_ARG_VALID;
constexpr auto test3_1b = CodeExpiresFeature::details::DATE_FORMAT_COMPILER_VALID;
constexpr auto test3_1c = CodeExpiresFeature::details::CheckArg("hello");
constexpr auto test3_2a = CodeExpiresFeature::details::COMPILE_DATE;
constexpr auto test3_2b = CodeExpiresFeature::details::ArgDate("hello");
constexpr auto test3_3a = CodeExpiresFeature::ExpiresConditionCheck("hello");

__EXPIRES__("20191326");
constexpr auto test4_1a = CodeExpiresFeature::details::DATE_FORMAT_ARG_VALID;
constexpr auto test4_1b = CodeExpiresFeature::details::DATE_FORMAT_COMPILER_VALID;
constexpr auto test4_1c = CodeExpiresFeature::details::CheckArg("20191326");
constexpr auto test4_2a = CodeExpiresFeature::details::COMPILE_DATE;
constexpr auto test4_2b = CodeExpiresFeature::details::ArgDate("20191326");
constexpr auto test4_3a = CodeExpiresFeature::ExpiresConditionCheck("20191326");

constexpr char* date = "20200202";
__EXPIRES__(date);
constexpr auto test5_1a = CodeExpiresFeature::details::DATE_FORMAT_ARG_VALID;
constexpr auto test5_1b = CodeExpiresFeature::details::DATE_FORMAT_COMPILER_VALID;
constexpr auto test5_1c = CodeExpiresFeature::details::CheckArg(date);
constexpr auto test5_2a = CodeExpiresFeature::details::COMPILE_DATE;
constexpr auto test5_2b = CodeExpiresFeature::details::ArgDate(date);
constexpr auto test5_3a = CodeExpiresFeature::ExpiresConditionCheck(date);

constexpr auto test6_1 = CodeExpiresFeature::details::ce_strlen("20190801");
constexpr auto test6_2 = CodeExpiresFeature::details::FindFirst_s("20190801", 9, '9');
constexpr auto test6_3 = CodeExpiresFeature::details::FindLast_s("20190801", 9, '1');
constexpr auto test6_4 = CodeExpiresFeature::details::FindLast_s("20190801", 2, '1');
constexpr auto test6_5 = CodeExpiresFeature::details::FindLast_s("20190801", 9, 'X');

constexpr char datefmt[] = "MMM-dd-YYYY";
constexpr auto test7_1 = CodeExpiresFeature::details::GetYearStartIdx(datefmt);
constexpr auto test7_2 = CodeExpiresFeature::details::GetMonthStartIdx(datefmt);
constexpr auto test7_3 = CodeExpiresFeature::details::IsYearTwoDigitsLong(datefmt);
constexpr auto test7_4 = CodeExpiresFeature::details::IsMonthNumeric(datefmt);
constexpr auto test7_5 = CodeExpiresFeature::details::IsMonthZeroPadded(datefmt);
constexpr auto test7_6 = CodeExpiresFeature::details::GetDayStartIdx(datefmt);
constexpr auto test7_7 = CodeExpiresFeature::details::IsDayZeroPadded(datefmt);
constexpr auto test7_8 = CodeExpiresFeature::details::ExtractNum("hello04", 5, true);

constexpr auto test8_1 = CodeExpiresFeature::details::ExtractYear("Jul  3, 1820", 8, false);
constexpr auto test8_2 = CodeExpiresFeature::details::ExtractYear("Jul  3, 1820", 3, false);
constexpr auto test8_3 = CodeExpiresFeature::details::ExtractYear("Jul  3, 1820", 8, true);
constexpr auto test8_4 = CodeExpiresFeature::details::ExtractYear("Jul  3, 1820", 10, true);
constexpr auto test8_5 = CodeExpiresFeature::details::ExtractYear("Jul  3, 1820", 100, true);

constexpr auto test9_1 = CodeExpiresFeature::details::ExtractMonth("Jul  3, 1820", 0, false, false);
constexpr auto test9_2 = CodeExpiresFeature::details::ExtractMonth("Jul  3, 1820", 0, true, false);
constexpr auto test9_3 = CodeExpiresFeature::details::ExtractMonth("Jul  3, 1820", 0, true, true);

constexpr auto test9_4 = CodeExpiresFeature::details::ExtractMonth("08  3, 1820", 0, false, false);
constexpr auto test9_5 = CodeExpiresFeature::details::ExtractMonth("08  3, 1820", 0, true, false);
constexpr auto test9_6 = CodeExpiresFeature::details::ExtractMonth("08  3, 1820", 0, true, true);

constexpr auto test9_7 = CodeExpiresFeature::details::ExtractMonth(" 8  3, 1820", 0, false, false);
constexpr auto test9_8 = CodeExpiresFeature::details::ExtractMonth(" 8  3, 1820", 0, true, false);
constexpr auto test9_9 = CodeExpiresFeature::details::ExtractMonth(" 8  3, 1820", 0, true, true);

constexpr auto test9_X = CodeExpiresFeature::details::ExtractMonth("8   3, 1820", 0, true, false); // !

constexpr auto test10_1 = CodeExpiresFeature::details::ExtractDay("Jul  3, 1820", 4, false);
constexpr auto test10_2 = CodeExpiresFeature::details::ExtractDay("Jul 03, 1820", 4, false);
constexpr auto test10_3 = CodeExpiresFeature::details::ExtractDay("Jul  3, 1820", 4, true);
constexpr auto test10_4 = CodeExpiresFeature::details::ExtractDay("Jul 03, 1820", 4, true);

