//------------------------------------------------------------------------------
/*
    Regression test for AMMHelpers adjustAmountsByLPTokens bug:
    Ensures withdrawByTokens uses the actual LP token amount.
*/

#include <ripple/app/misc/AMMHelpers.h>
#include <ripple/basics/Number.h>
#include <ripple/protocol/Issue.h>
#include <ripple/protocol/STAmount.h>
#include <ripple/beast/unit_test.h>

namespace ripple {
namespace test {

class AMMCalc_test : public beast::unit_test::suite
{
public:
    void test_adjustAmountsByLPTokens_regression()
    {
        // Dummy asset and LP token issues
        Issue assetIssue{{"rrrrrrrrrrrrrrrrrrrrrhoLvTp", 0}, 0};
        Issue lpIssue{{"rrrrrrrrrrrrrrrrrrrrrhoLvTp", 0}, 1};

        // Asset and LP total balances
        STAmount assetBalance(assetIssue, 1000); // e.g. 1000 tokens
        STAmount lpTotal(lpIssue, 1000);         // 1000 LP tokens exist

        // User requests to withdraw 333.33 LP tokens, but rounding gives 333
        STAmount lpRequest(lpIssue, 333.33); // simulate non-integer, but STAmount stores as integer
        STAmount lpRequestInt(lpIssue, 333); // what will actually be used

        // Use a nominal fee
        std::uint16_t tfee = 0;

        // Call adjustAmountsByLPTokens (withdrawal, so isDeposit = false)
        auto [amountActual, amount2Actual, lpTokensActual] =
            adjustAmountsByLPTokens(
                assetBalance,              // amountBalance
                assetBalance,              // amount (arbitrary for single asset)
                std::nullopt,              // amount2
                lpTotal,                   // lptAMMBalance
                lpRequest,                 // lpTokens
                tfee,
                false                      // isDeposit
            );

        // The actual LP tokens used should be <= requested due to rounding
        BEAST_EXPECT(lpTokensActual <= lpRequest);

        // Compute expected proportional withdrawal
        // amountExpected = assetBalance * lpTokensActual / lpTotal
        auto amountExpected = toSTAmount(
            assetBalance.issue(),
            ripple::Number(lpTokensActual) * ripple::Number(assetBalance) / ripple::Number(lpTotal)
        );

        // The amount withdrawn should be <= expected proportional amount
        BEAST_EXPECT(amountActual <= amountExpected);
    }

    void run() override
    {
        test_adjustAmountsByLPTokens_regression();
    }
};

BEAST_DEFINE_TESTSUITE(AMMCalc, app, ripple);

} // test
} // ripple