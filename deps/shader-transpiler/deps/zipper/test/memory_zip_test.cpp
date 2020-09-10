#include "catch.hpp"

#include <zipper/zipper.h>
#include <zipper/unzipper.h>
#include <zipper/tools.h>

#include <vector>
#include <fstream>
#include <ostream>
#include <string>

using namespace zipper;

SCENARIO("zip vector feed with different inputs", "[zip]")
{
    GIVEN("A Zip outputed to a vector")
    {
        std::vector<unsigned char> zipvec;
        zipper::Zipper zipper(zipvec);

        WHEN("a file containing 'test file compression' is added and named 'test1'")
        {
            std::ofstream test1("test1.txt");
            test1 << "test file compression";
            test1.flush();
            test1.close();

            std::ifstream test1stream("test1.txt");

            zipper.add(test1stream, "test1.txt");

            test1stream.close();
            zipper.close();

            std::remove("test1.txt");

            zipper::Unzipper unzipper(zipvec);

            THEN("the zip vector has one entry named 'test1.txt'")
            {
                REQUIRE(unzipper.entries().size() == 1);
                REQUIRE(unzipper.entries().front().name == "test1.txt");

                AND_THEN("extracting the test1.txt entry creates a file named 'test1.txt' with the text 'test file compression'")
                {
                    unzipper.extractEntry("test1.txt");
                    // due to sections forking or creating different stacks we need to make sure the local instance is closed to
                    // prevent mixing the closing when both instances are freed at the end of the scope
                    unzipper.close();

                    REQUIRE(checkFileExists("test1.txt"));

                    std::ifstream testfile("test1.txt");
                    REQUIRE(testfile.good());

                    std::string test((std::istreambuf_iterator<char>(testfile)), std::istreambuf_iterator<char>());
                    testfile.close();
                    REQUIRE(test == "test file compression");

                    AND_WHEN("another file containing 'other data to compression test' and named 'test2.dat' is added inside a folder 'TestFolder'")
                    {
                        std::ofstream test2("test2.dat");
                        test2 << "other data to compression test";
                        test2.flush();
                        test2.close();

                        std::ifstream test2stream("test2.dat");

                        zipper.open();
                        zipper.add(test2stream, "TestFolder/test2.dat");
                        zipper.close();

                        test2stream.close();
                        std::remove("test2.dat");

                        zipper::Unzipper unzipper(zipvec);

                        THEN("the zip vector has two entries named 'test1.txt' and 'TestFolder/test2.dat'")
                        {
                            REQUIRE(unzipper.entries().size() == 2);
                            REQUIRE(unzipper.entries().front().name == "test1.txt");
                            REQUIRE(unzipper.entries()[1].name == "TestFolder/test2.dat");

                            AND_THEN("extracting the test2.dat entry creates a folder 'TestFolder' with a file named 'test2.dat' with the text 'other data to compression test'")
                            {
                                unzipper.extract();

                                REQUIRE(checkFileExists("TestFolder/test2.dat"));

                                std::ifstream testfile("TestFolder/test2.dat");
                                REQUIRE(testfile.good());

                                std::string test((std::istreambuf_iterator<char>(testfile)), std::istreambuf_iterator<char>());
                                testfile.close();
                                REQUIRE(test == "other data to compression test");

                                AND_THEN("extracting the test2.dat entry to memory fills a vector with 'other data to compression test'")
                                {
                                    std::vector<unsigned char> resvec;
                                    unzipper.extractEntryToMemory("TestFolder/test2.dat", resvec);
                                    unzipper.close();

                                    std::string test(resvec.begin(), resvec.end());

                                    REQUIRE(test == "other data to compression test");
                                }

                                unzipper.close();
                            }
                        }

                        std::remove("test1.txt");
                        removeFolder("TestFolder");
                    }
                }
            }

            zipvec.clear();
        }

        WHEN("a stringstream containing 'test string data compression' is added and named 'strdata'")
        {
            std::stringstream strdata;
            strdata << "test string data compression";

            zipper.add(strdata, "strdata");
            zipper.close();

            zipper::Unzipper unzipper(zipvec);

            THEN("the zip vector has one entry named 'strdata'")
            {
                REQUIRE(unzipper.entries().size() == 1);
                REQUIRE(unzipper.entries().front().name == "strdata");

                AND_THEN("extracting the strdata entry creates a file named 'strdata' with the txt 'test string data compression'")
                {
                    unzipper.extract();

                    REQUIRE(checkFileExists("strdata"));

                    std::ifstream testfile("strdata");
                    REQUIRE(testfile.good());

                    std::string test((std::istreambuf_iterator<char>(testfile)), std::istreambuf_iterator<char>());
                    testfile.close();
                    REQUIRE(test == "test string data compression");

                    AND_THEN("extracting the strdata entry to memory, fills a vector with the txt 'test string data compression'")
                    {
                        std::vector<unsigned char> resvec;
                        unzipper.extractEntryToMemory("strdata", resvec);
                        unzipper.close();

                        std::string test(resvec.begin(), resvec.end());

                        REQUIRE(test == "test string data compression");
                    }

                    unzipper.close();
                }
            }

            std::remove("strdata");
            zipvec.clear();
        }

        WHEN("a file containing 'test file compression' is added and named 'test1' in subdirectory 'subdirectory'")
        {
            bool fileWasCreated = zipper::makedir("subdirectory");

            REQUIRE(fileWasCreated);

            std::ofstream test1("./subdirectory/test1.txt");
            test1 << "test file compression";
            test1.flush();
            test1.close();


            auto flags = Zipper::zipFlags::Better | Zipper::zipFlags::SaveHierarchy;
            zipper.add("./subdirectory/test1.txt", static_cast<Zipper::zipFlags>(flags));

            zipper.close();

            std::remove("./subdirectory/test1.txt");

            zipper::Unzipper unzipper(zipvec);

            THEN("the zip vector has entry named './subdirectory/test1.txt'")
            {
                REQUIRE(unzipper.entries().size() == 1);
                REQUIRE(unzipper.entries().front().name == "./subdirectory/test1.txt");

                AND_THEN("extracting the test1.txt entry creates a file named 'test1.txt' with the text 'test file compression'")
                {
                    unzipper.extractEntry("./subdirectory/test1.txt");
                    // due to sections forking or creating different stacks we need to make sure the local instance is closed to
                    // prevent mixing the closing when both instances are freed at the end of the scope
                    unzipper.close();

                    REQUIRE(checkFileExists("./subdirectory/test1.txt"));

                    std::ifstream testfile("./subdirectory/test1.txt");
                    REQUIRE(testfile.good());

                    std::string test((std::istreambuf_iterator<char>(testfile)), std::istreambuf_iterator<char>());
                    testfile.close();
                    REQUIRE(test == "test file compression");

                    removeFolder("subdirectory");
                }
            }

            zipvec.clear();
        }
    }
}
