#ifndef TESTBED_ANALYSIZER_H
#define TESTBED_ANALYSIZER_H

#include <map>
#include <vector>

#include "Identifier.h"
#include "Skeleton.h"
#include "SceneDemo.h"
#include "Event.h"

using namespace identifier;
using namespace skeleton;

namespace analysizer {
    class Analysizer {
    private:
        // ------------------------- Constants ----------------------- //
        const std::map<std::string, std::map<std::string, std::vector<std::string>>> identifier_target_list = {
                {"forehand_stroke", {{"rotation", {"hip"}}, {"fore_arm", {"rForeArm"}}}},
        };

        const std::map<std::string, std::map<std::string, std::string>> identifier_notPass_suggestions = {
                {"forehand_stroke",
                 {{"rotation", "Twist your waist correctly."}, {"fore_arm", "Wave your arm correctly."}}},
        };

        // ------------------------- Attributes ----------------------- //
        std::string analysizer_name;

        Skeleton *target_skeleton;

        std::map<std::string, Identifier *> identifiers;

        Identifier *output_identifier;

        std::string output_filename;

        std::vector<std::string> identifier_name_list;

        std::map<std::string, bool> identifier_pass_list;

        // ------------------------- Methods ----------------------- //
        void _Analyse(map<string, Identifier *> &identifier_list, const string &openposePath);
    public:
        // ------------------------- Events ----------------------- //
        event::Event<> analysize_done;

        // ------------------------- Methods ----------------------- //
        Analysizer(Skeleton *skeleton);

        Analysizer(Skeleton *target_skeleton, const std::string &analysizer_name);

        ~Analysizer();

        /**
         * Analyze the skeleton with all the identifiers
         */
        void Analyze(const string &openposePath);

        /**
         * Analyze the skeleton with the given identifier
         * @param identifier
         */
        void Analyze(Identifier *identifier, const string &openposePath);

        string Suggest_str(const string &identifier_name);

        string Suggest_str();
    };

}


#endif //TESTBED_ANALYSIZER_H
