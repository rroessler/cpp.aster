/// Vendor Includes
#include <aster/aster.hpp>
#include <catch2/catch_all.hpp>
#include <print>

//  TEST CASES  //

TEST_CASE("Glob::Match") {
    SECTION("empty") {
        CHECK(Aster::Match::empty("", ""));
        CHECK(!Aster::Match::empty("", "abc"));
    }

    SECTION("glob") {
        CHECK(Aster::Match::glob("abc", "abc"));
        CHECK(Aster::Match::glob("*", "abc"));
        CHECK(Aster::Match::glob("*", ""));
        CHECK(Aster::Match::glob("**", ""));
        CHECK(Aster::Match::glob("*c", "abc"));
        CHECK(!Aster::Match::glob("*b", "abc"));
        CHECK(Aster::Match::glob("a*", "abc"));
        CHECK(!Aster::Match::glob("b*", "abc"));
        CHECK(Aster::Match::glob("a*", "a"));
        CHECK(Aster::Match::glob("*a", "a"));
        CHECK(Aster::Match::glob("a*b*c*d*e*", "axbxcxdxe"));
        CHECK(Aster::Match::glob("a*b*c*d*e*", "axbxcxdxexxx"));
        CHECK(Aster::Match::glob("a*b?c*x", "abxbbxdbxebxczzx"));
        CHECK(!Aster::Match::glob("a*b?c*x", "abxbbxdbxebxczzy"));

        CHECK(!Aster::Match::glob("!*", "abc"));
        CHECK(!Aster::Match::glob("!*", ""));
        CHECK(Aster::Match::glob("!*b", "abc"));

        CHECK(!Aster::Match::glob("a!!b", "a"));
        CHECK(!Aster::Match::glob("a!!b", "aa"));
        CHECK(!Aster::Match::glob("a!!b", "a/b"));
        CHECK(!Aster::Match::glob("a!!b", "a!b"));
        CHECK(Aster::Match::glob("a!!b", "a!!b"));
        CHECK(!Aster::Match::glob("a!!b", "a/!!/b"));

        CHECK(!Aster::Match::glob("!abc", "abc"));
        CHECK(Aster::Match::glob("!!abc", "abc"));
        CHECK(!Aster::Match::glob("!!!abc", "abc"));
        CHECK(Aster::Match::glob("!!!!abc", "abc"));
        CHECK(!Aster::Match::glob("!!!!!abc", "abc"));
        CHECK(Aster::Match::glob("!!!!!!abc", "abc"));
        CHECK(!Aster::Match::glob("!!!!!!!abc", "abc"));
        CHECK(Aster::Match::glob("!!!!!!!!abc", "abc"));

        CHECK(Aster::Match::glob("a/*/test", "a/foo/test"));
        CHECK(!Aster::Match::glob("a/*/test", "a/foo/bar/test"));
        CHECK(Aster::Match::glob("a/**/test", "a/foo/test"));
        CHECK(Aster::Match::glob("a/**/test", "a/foo/bar/test"));
        CHECK(Aster::Match::glob("a/**/b/c", "a/foo/bar/b/c"));
        CHECK(Aster::Match::glob("a\\*b", "a*b"));
        CHECK(!Aster::Match::glob("a\\*b", "axb"));

        CHECK(Aster::Match::glob("[abc]", "a"));
        CHECK(Aster::Match::glob("[abc]", "b"));
        CHECK(Aster::Match::glob("[abc]", "c"));
        CHECK(!Aster::Match::glob("[abc]", "d"));
        CHECK(Aster::Match::glob("x[abc]x", "xax"));
        CHECK(Aster::Match::glob("x[abc]x", "xbx"));
        CHECK(Aster::Match::glob("x[abc]x", "xcx"));
        CHECK(!Aster::Match::glob("x[abc]x", "xdx"));
        CHECK(!Aster::Match::glob("x[abc]x", "xay"));
        CHECK(Aster::Match::glob("[?]", "?"));
        CHECK(!Aster::Match::glob("[?]", "a"));
        CHECK(Aster::Match::glob("[*]", "*"));
        CHECK(!Aster::Match::glob("[*]", "a"));

        CHECK(Aster::Match::glob("[a-cx]", "a"));
        CHECK(Aster::Match::glob("[a-cx]", "b"));
        CHECK(Aster::Match::glob("[a-cx]", "c"));
        CHECK(!Aster::Match::glob("[a-cx]", "d"));
        CHECK(Aster::Match::glob("[a-cx]", "x"));

        CHECK(!Aster::Match::glob("[^abc]", "a"));
        CHECK(!Aster::Match::glob("[^abc]", "b"));
        CHECK(!Aster::Match::glob("[^abc]", "c"));
        CHECK(Aster::Match::glob("[^abc]", "d"));
        CHECK(!Aster::Match::glob("[!abc]", "a"));
        CHECK(!Aster::Match::glob("[!abc]", "b"));
        CHECK(!Aster::Match::glob("[!abc]", "c"));
        CHECK(Aster::Match::glob("[!abc]", "d"));
        CHECK(Aster::Match::glob("[\\!]", "!"));

        CHECK(Aster::Match::glob("a*b*[cy]*d*e*", "axbxcxdxexxx"));
        CHECK(Aster::Match::glob("a*b*[cy]*d*e*", "axbxyxdxexxx"));
        CHECK(Aster::Match::glob("a*b*[cy]*d*e*", "axbxxxyxdxexxx"));

        CHECK(Aster::Match::glob("test.{jpg,png}", "test.jpg"));
        CHECK(Aster::Match::glob("test.{jpg,png}", "test.png"));
        CHECK(Aster::Match::glob("test.{j*g,p*g}", "test.jpg"));
        CHECK(Aster::Match::glob("test.{j*g,p*g}", "test.jpxxxg"));
        CHECK(Aster::Match::glob("test.{j*g,p*g}", "test.jxg"));
        CHECK(!Aster::Match::glob("test.{j*g,p*g}", "test.jnt"));

        CHECK(Aster::Match::glob("test.{j*g,j*c}", "test.jnc"));
        CHECK(Aster::Match::glob("test.{jpg,p*g}", "test.png"));
        CHECK(Aster::Match::glob("test.{jpg,p*g}", "test.pxg"));
        CHECK(!Aster::Match::glob("test.{jpg,p*g}", "test.pnt"));
        CHECK(Aster::Match::glob("test.{jpeg,png}", "test.jpeg"));
        CHECK(!Aster::Match::glob("test.{jpeg,png}", "test.jpg"));
        CHECK(Aster::Match::glob("test.{jpeg,png}", "test.png"));
        CHECK(Aster::Match::glob("test.{jp\\,g,png}", "test.jp,g"));
        CHECK(!Aster::Match::glob("test.{jp\\,g,png}", "test.jxg"));
        CHECK(Aster::Match::glob("test/{foo,bar}/baz", "test/foo/baz"));
        CHECK(Aster::Match::glob("test/{foo,bar}/baz", "test/bar/baz"));
        CHECK(!Aster::Match::glob("test/{foo,bar}/baz", "test/baz/baz"));
        CHECK(Aster::Match::glob("test/{foo*,bar*}/baz", "test/foooooo/baz"));
        CHECK(Aster::Match::glob("test/{foo*,bar*}/baz", "test/barrrrr/baz"));
        CHECK(Aster::Match::glob("test/{*foo,*bar}/baz", "test/xxxxfoo/baz"));
        CHECK(Aster::Match::glob("test/{*foo,*bar}/baz", "test/xxxxbar/baz"));
        CHECK(Aster::Match::glob("test/{foo/**,bar}/baz", "test/bar/baz"));
        CHECK(!Aster::Match::glob("test/{foo/**,bar}/baz", "test/bar/test/baz"));

        CHECK(!Aster::Match::glob("*.txt", "some/path/to/the/needle.txt"));
        CHECK(Aster::Match::glob("some/**/needle.{js,ts,txt}", "some/a/path/to/the/needle.txt"));
        CHECK(Aster::Match::glob("some/**/{a,b,c}/**/needle.txt", "some/foo/a/path/to/the/needle.txt"));
        CHECK(!Aster::Match::glob("some/**/{a,b,c}/**/needle.txt", "some/foo/d/path/to/the/needle.txt"));

        CHECK(Aster::Match::glob("a/{a{a,b},b}", "a/aa"));
        CHECK(Aster::Match::glob("a/{a{a,b},b}", "a/ab"));
        CHECK(!Aster::Match::glob("a/{a{a,b},b}", "a/ac"));
        CHECK(Aster::Match::glob("a/{a{a,b},b}", "a/b"));
        CHECK(!Aster::Match::glob("a/{a{a,b},b}", "a/c"));
        CHECK(Aster::Match::glob("a/{b,c[}]*}", "a/b"));
        CHECK(Aster::Match::glob("a/{b,c[}]*}", "a/c}xx"));

        CHECK(Aster::Match::glob("/**/*a", "/a/a"));
        CHECK(Aster::Match::glob("**/*.js", "a/b.c/c.js"));
        CHECK(Aster::Match::glob("**/**/*.js", "a/b.c/c.js"));
        CHECK(Aster::Match::glob("a/**/*.d", "a/b/c.d"));
        CHECK(Aster::Match::glob("a/**/*.d", "a/.b/c.d"));

        CHECK(Aster::Match::glob("**/*/**", "a/b/c"));
        CHECK(Aster::Match::glob("**/*/c.js", "a/b/c.js"));
    }
}

TEST_CASE("Glob::Walker") {
    BENCHMARK("Baseline") {
        auto walker = Aster::Walker("*.md");
        for (const auto& _ : walker.iterate()) {}
    };

    BENCHMARK("Recursive") {
        auto walker = Aster::Walker("**/*.md");
        for (const auto& _ : walker.iterate()) {}
    };

    BENCHMARK("Dynamic") {
        auto walker = Aster::Walker("**/*");
        for (const auto& _ : walker.iterate()) {}
    };
}

TEST_CASE("Glob::Pattern") {
    std::string_view globstar = "**/*";  // prepare a fast-globstar
    std::string_view input = "some/small/or/large/path/to/a/needle.txt";
    auto pattern = Aster::Pattern(globstar);  // prepare the compiled pattern

    // pre-ensure the glob and pattern match
    CHECK(Aster::Match::glob(globstar, input));
    CHECK(pattern.matches(input));

    // and then coordinate running benchmarks
    BENCHMARK("Match::glob") { return Aster::Match::glob(globstar, input); };
    BENCHMARK("Pattern::matches") { return pattern.matches(input); };
}

//  TEST RUNNER  //

/// @brief Handles entry for testing.
int32_t main(int32_t argc, char** argv) { return Catch::Session().run(argc, argv); }
