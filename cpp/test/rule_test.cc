// Copyright (C) 2013 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "rule.h"

#include <libaddressinput/address_field.h>
#include <libaddressinput/localization.h>

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "address_field_util.h"
#include "grit.h"
#include "messages.h"
#include "region_data_constants.h"

namespace {

using i18n::addressinput::AddressField;
using i18n::addressinput::ADMIN_AREA;
using i18n::addressinput::INVALID_MESSAGE_ID;
using i18n::addressinput::LOCALITY;
using i18n::addressinput::Localization;
using i18n::addressinput::NEWLINE;
using i18n::addressinput::POSTAL_CODE;
using i18n::addressinput::RECIPIENT;
using i18n::addressinput::RegionDataConstants;
using i18n::addressinput::Rule;
using i18n::addressinput::STREET_ADDRESS;

TEST(RuleTest, CopyOverwritesRule) {
  Rule rule;
  ASSERT_TRUE(rule.ParseSerializedRule("{"
                                       "\"fmt\":\"%S%Z\","
                                       "\"lfmt\":\"%Z%S\","
                                       "\"id\":\"data/XA\","
                                       "\"lname\":\"Testistan\","
                                       "\"require\":\"AC\","
                                       "\"sub_keys\":\"aa~bb~cc\","
                                       "\"languages\":\"en~fr\","
                                       "\"zip\":\"\\\\d{3}\","
                                       "\"state_name_type\":\"area\","
                                       "\"zip_name_type\":\"postal\""
                                       "}"));

  Rule copy;
  EXPECT_NE(rule.GetFormat(), copy.GetFormat());
  EXPECT_NE(rule.GetLatinFormat(), copy.GetLatinFormat());
  EXPECT_NE(rule.GetId(), copy.GetId());
  EXPECT_NE(rule.GetRequired(), copy.GetRequired());
  EXPECT_NE(rule.GetSubKeys(), copy.GetSubKeys());
  EXPECT_NE(rule.GetLanguages(), copy.GetLanguages());
  EXPECT_NE(rule.GetAdminAreaNameMessageId(),
            copy.GetAdminAreaNameMessageId());
  EXPECT_NE(rule.GetPostalCodeNameMessageId(),
            copy.GetPostalCodeNameMessageId());

  EXPECT_TRUE(rule.GetPostalCodeMatcher() != NULL);
  EXPECT_TRUE(copy.GetPostalCodeMatcher() == NULL);

  copy.CopyFrom(rule);
  EXPECT_EQ(rule.GetFormat(), copy.GetFormat());
  EXPECT_EQ(rule.GetLatinFormat(), copy.GetLatinFormat());
  EXPECT_EQ(rule.GetId(), copy.GetId());
  EXPECT_EQ(rule.GetRequired(), copy.GetRequired());
  EXPECT_EQ(rule.GetSubKeys(), copy.GetSubKeys());
  EXPECT_EQ(rule.GetLanguages(), copy.GetLanguages());
  EXPECT_EQ(rule.GetAdminAreaNameMessageId(),
            copy.GetAdminAreaNameMessageId());
  EXPECT_EQ(rule.GetPostalCodeNameMessageId(),
            copy.GetPostalCodeNameMessageId());

  EXPECT_TRUE(copy.GetPostalCodeMatcher() != NULL);
}

TEST(RuleTest, ParseOverwritesRule) {
  Rule rule;
  ASSERT_TRUE(rule.ParseSerializedRule("{"
                                       "\"fmt\":\"%S%Z\","
                                       "\"state_name_type\":\"area\","
                                       "\"zip_name_type\":\"postal\""
                                       "}"));
  EXPECT_FALSE(rule.GetFormat().empty());
  EXPECT_EQ(IDS_LIBADDRESSINPUT_AREA,
            rule.GetAdminAreaNameMessageId());
  EXPECT_EQ(IDS_LIBADDRESSINPUT_POSTAL_CODE_LABEL,
            rule.GetPostalCodeNameMessageId());

  ASSERT_TRUE(rule.ParseSerializedRule("{"
                                       "\"fmt\":\"\","
                                       "\"state_name_type\":\"do_si\","
                                       "\"zip_name_type\":\"zip\""
                                       "}"));
  EXPECT_TRUE(rule.GetFormat().empty());
  EXPECT_EQ(IDS_LIBADDRESSINPUT_DO_SI,
            rule.GetAdminAreaNameMessageId());
  EXPECT_EQ(IDS_LIBADDRESSINPUT_ZIP_CODE_LABEL,
            rule.GetPostalCodeNameMessageId());
}

TEST(RuleTest, ParsesFormatCorrectly) {
  std::vector<AddressField> expected;
  expected.push_back(ADMIN_AREA);
  expected.push_back(LOCALITY);
  Rule rule;
  ASSERT_TRUE(rule.ParseSerializedRule("{\"fmt\":\"%S%C\"}"));
  EXPECT_EQ(expected, rule.GetFormat());
}

TEST(RuleTest, ParsesLatinFormatCorrectly) {
  std::vector<AddressField> expected;
  expected.push_back(LOCALITY);
  expected.push_back(ADMIN_AREA);
  Rule rule;
  ASSERT_TRUE(rule.ParseSerializedRule("{\"lfmt\":\"%C%S\"}"));
  EXPECT_EQ(expected, rule.GetLatinFormat());
}

TEST(RuleTest, ParsesRequiredCorrectly) {
  std::vector<AddressField> expected;
  expected.push_back(STREET_ADDRESS);
  expected.push_back(LOCALITY);
  Rule rule;
  ASSERT_TRUE(rule.ParseSerializedRule("{\"require\":\"AC\"}"));
  EXPECT_EQ(expected, rule.GetRequired());
}

TEST(RuleTest, ParsesSubKeysCorrectly) {
  std::vector<std::string> expected;
  expected.push_back("aa");
  expected.push_back("bb");
  expected.push_back("cc");
  Rule rule;
  ASSERT_TRUE(rule.ParseSerializedRule("{\"sub_keys\":\"aa~bb~cc\"}"));
  EXPECT_EQ(expected, rule.GetSubKeys());
}

TEST(RuleTest, ParsesLanguagesCorrectly) {
  std::vector<std::string> expected;
  expected.push_back("de");
  expected.push_back("fr");
  expected.push_back("it");
  Rule rule;
  ASSERT_TRUE(rule.ParseSerializedRule("{\"languages\":\"de~fr~it\"}"));
  EXPECT_EQ(expected, rule.GetLanguages());
}

TEST(RuleTest, PostalCodeMatcher) {
  Rule rule;
  ASSERT_TRUE(rule.ParseSerializedRule("{\"zip\":\"\\\\d{3}\"}"));
  EXPECT_TRUE(rule.GetPostalCodeMatcher() != NULL);
}

TEST(RuleTest, PostalCodeMatcherInvalidRegExp) {
  Rule rule;
  ASSERT_TRUE(rule.ParseSerializedRule("{\"zip\":\"(\"}"));
  EXPECT_TRUE(rule.GetPostalCodeMatcher() == NULL);
}

TEST(RuleTest, EmptyStringIsNotValid) {
  Rule rule;
  EXPECT_FALSE(rule.ParseSerializedRule(std::string()));
}

TEST(RuleTest, EmptyDictionaryIsValid) {
  Rule rule;
  EXPECT_TRUE(rule.ParseSerializedRule("{}"));
}

// Tests for parsing the postal code name.
class PostalCodeNameParseTest
    : public testing::TestWithParam<std::pair<std::string, int> > {
 protected:
  Rule rule_;
};

// Verifies that a postal code name is parsed correctly.
TEST_P(PostalCodeNameParseTest, ParsedCorrectly) {
  ASSERT_TRUE(rule_.ParseSerializedRule(GetParam().first));
  EXPECT_EQ(GetParam().second, rule_.GetPostalCodeNameMessageId());
}

// Test parsing all postal code names.
INSTANTIATE_TEST_CASE_P(
    AllPostalCodeNames, PostalCodeNameParseTest,
    testing::Values(
        std::make_pair("{\"zip_name_type\":\"postal\"}",
                       IDS_LIBADDRESSINPUT_POSTAL_CODE_LABEL),
        std::make_pair("{\"zip_name_type\":\"zip\"}",
                       IDS_LIBADDRESSINPUT_ZIP_CODE_LABEL)));

// Tests for parsing the administrative area name.
class AdminAreaNameParseTest
    : public testing::TestWithParam<std::pair<std::string, int> > {
 protected:
  Rule rule_;
};

// Verifies that an administrative area name is parsed correctly.
TEST_P(AdminAreaNameParseTest, ParsedCorrectly) {
  ASSERT_TRUE(rule_.ParseSerializedRule(GetParam().first));
  EXPECT_EQ(GetParam().second, rule_.GetAdminAreaNameMessageId());
}

// Test parsing all administrative area names.
INSTANTIATE_TEST_CASE_P(
    AllAdminAreaNames, AdminAreaNameParseTest,
    testing::Values(
        std::make_pair("{\"state_name_type\":\"area\"}",
                       IDS_LIBADDRESSINPUT_AREA),
        std::make_pair("{\"state_name_type\":\"county\"}",
                       IDS_LIBADDRESSINPUT_COUNTY),
        std::make_pair("{\"state_name_type\":\"department\"}",
                       IDS_LIBADDRESSINPUT_DEPARTMENT),
        std::make_pair("{\"state_name_type\":\"district\"}",
                       IDS_LIBADDRESSINPUT_DISTRICT),
        std::make_pair("{\"state_name_type\":\"do_si\"}",
                       IDS_LIBADDRESSINPUT_DO_SI),
        std::make_pair("{\"state_name_type\":\"emirate\"}",
                       IDS_LIBADDRESSINPUT_EMIRATE),
        std::make_pair("{\"state_name_type\":\"island\"}",
                       IDS_LIBADDRESSINPUT_ISLAND),
        std::make_pair("{\"state_name_type\":\"parish\"}",
                       IDS_LIBADDRESSINPUT_PARISH),
        std::make_pair("{\"state_name_type\":\"prefecture\"}",
                       IDS_LIBADDRESSINPUT_PREFECTURE),
        std::make_pair("{\"state_name_type\":\"province\"}",
                       IDS_LIBADDRESSINPUT_PROVINCE),
        std::make_pair("{\"state_name_type\":\"state\"}",
                       IDS_LIBADDRESSINPUT_STATE)));

// Tests for rule parsing.
class RuleParseTest : public testing::TestWithParam<std::string> {
 protected:
  const std::string& GetRegionData() const {
    // GetParam() is either a region code or the region data itself.
    // RegionDataContants::GetRegionData() returns an empty string for anything
    // that's not a reigon code.
    const std::string& data = RegionDataConstants::GetRegionData(GetParam());
    return !data.empty() ? data : GetParam();
  }

  Rule rule_;
  Localization localization_;
};

// Verifies that a region data can be parsed successfully.
TEST_P(RuleParseTest, RegionDataParsedSuccessfully) {
  EXPECT_TRUE(rule_.ParseSerializedRule(GetRegionData()));
}

// Verifies that the admin area name type corresponds to a UI string.
TEST_P(RuleParseTest, AdminAreaNameTypeHasUiString) {
  const std::string& region_data = GetRegionData();
  rule_.ParseSerializedRule(region_data);
  if (region_data.find("state_name_type") != std::string::npos) {
    EXPECT_NE(INVALID_MESSAGE_ID, rule_.GetAdminAreaNameMessageId());
    EXPECT_FALSE(
        localization_.GetString(rule_.GetAdminAreaNameMessageId()).empty());
  }
}

// Verifies that the postal code name type corresponds to a UI string.
TEST_P(RuleParseTest, PostalCodeNameTypeHasUiString) {
  const std::string& region_data = GetRegionData();
  rule_.ParseSerializedRule(region_data);
  if (region_data.find("zip_name_type") != std::string::npos) {
    EXPECT_NE(INVALID_MESSAGE_ID, rule_.GetPostalCodeNameMessageId());
    EXPECT_FALSE(
        localization_.GetString(rule_.GetPostalCodeNameMessageId()).empty());
  }
}

// Test parsing all region data.
INSTANTIATE_TEST_CASE_P(
    AllRulesTest, RuleParseTest,
    testing::ValuesIn(RegionDataConstants::GetRegionCodes()));

// Test parsing the default rule.
INSTANTIATE_TEST_CASE_P(
    DefaultRuleTest, RuleParseTest,
    testing::Values(RegionDataConstants::GetDefaultRegionData()));

}  // namespace
