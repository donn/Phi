#!/usr/bin/env ruby
errors = 0
languages = {}
for localization in Dir["Localization/*"]
    i = 0
    name = File.basename(localization, '.*')
    file = File.read(localization)
    hash = {}
    for line in file.split("\n")
        i += 1
        if line.empty?
            next
        end
        line_regex = /^\s*"((?:[^"\\]|\\"|\\)*)"\s*=\s*"((?:[^"\\]|\\"|\\)*)"\s*;\s*$/
        match = line_regex.match(line)
        if match.nil?
            STDERR.puts "#{name}:#{i}: '#{line}`"
            errors += 1
        else
            hash[match[1]] = match[2]
        end
    end
    languages[name] = hash
end

if errors != 0
    STDERR.puts "Errors have occurred trying to process the localization files. The compilation will stop."
    exit 65
end

finalHashValue = ""
for (key, smallhash) in languages
    preliminaryHashValue = "{\"#{key}\", {"
    for (key, value) in smallhash
        preliminaryHashValue += "{\"#{key}\", \"#{value}\"},\n"
    end
    preliminaryHashValue.chomp! ",\n"
    preliminaryHashValue += "}}"
    finalHashValue += "#{preliminaryHashValue}, "
end
finalHashValue.chomp! ", "
file = <<HEREDOC
#ifndef _localization_h
#define _localization_h
#include <cstdlib>
#include <map>
#include <string>

std::map< std::string, std::map< std::string, std::string > > localizationMap = {
    #{finalHashValue}
};

std::string Localize(std::string localizable) {
    const char* ctype = getenv("LC_CTYPE");
    if (!ctype) {
        ctype = "en_US.UTF-8";
    }
    auto ctypeString = std::string(ctype);
    auto locale = ctypeString.substr(0, ctypeString.find("_"));
    auto hash = &localizationMap["en"];
    if (localizationMap.find(locale) != localizationMap.end()) {
        hash = &localizationMap[locale];
    }
    if (hash->find(localizable) != hash->end()) {
        return (*hash)[localizable];
    }
    return localizable;
}

#endif
HEREDOC
puts file