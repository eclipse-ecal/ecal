/* =========================== LICENSE =================================
 *
 * Copyright 2025 AUMOVIO and subsidiaries. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * =========================== LICENSE =================================
 */

 #include <google/protobuf/compiler/plugin.pb.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/descriptor.h>

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using google::protobuf::FileDescriptorProto;
using google::protobuf::DescriptorProto;
using google::protobuf::EnumDescriptorProto;
using google::protobuf::FieldDescriptorProto;
using google::protobuf::compiler::CodeGeneratorRequest;
using google::protobuf::compiler::CodeGeneratorResponse;

namespace {

std::string replace_all(std::string s, const std::string& from, const std::string& to) {
    if (from.empty()) return s;
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
    return s;
}

std::string header_name_for_proto(const std::string& proto_name) {
    // foo/bar/baz.proto -> foo/bar/baz.pbftags.hpp
    std::string out = proto_name;
    if (out.size() >= 6 && out.substr(out.size()-6) == ".proto")
        out.replace(out.size()-6, 6, ".pbftags.hpp");
    else
        out += ".pbftags.hpp";
    return out;
}

std::string sanitize_identifier(std::string id) {
    for (char& c : id) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) c = '_';
    }
    if (!id.empty() && std::isdigit(static_cast<unsigned char>(id[0]))) id = "_" + id;

    static const std::unordered_set<std::string> kCppKeywords = {
        "alignas","alignof","and","and_eq","asm","atomic_cancel","atomic_commit","atomic_noexcept",
        "auto","bitand","bitor","bool","break","case","catch","char","char8_t","char16_t","char32_t",
        "class","compl","concept","const","consteval","constexpr","constinit","const_cast","continue",
        "co_await","co_return","co_yield","decltype","default","delete","do","double","dynamic_cast",
        "else","enum","explicit","export","extern","false","float","for","friend","goto","if","inline",
        "int","long","mutable","namespace","new","noexcept","not","not_eq","nullptr","operator","or",
        "or_eq","private","protected","public","register","reinterpret_cast","requires","return",
        "short","signed","sizeof","static","static_assert","static_cast","struct","switch","template",
        "this","thread_local","throw","true","try","typedef","typeid","typename","union","unsigned",
        "using","virtual","void","volatile","wchar_t","while","xor","xor_eq"
    };
    if (kCppKeywords.count(id)) id += "_";
    return id;
}

std::string to_snake(std::string s) {
    std::string out;
    out.reserve(s.size() * 2);
    bool prev_lower = false;
    for (char c : s) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            if (std::isupper(static_cast<unsigned char>(c))) {
                if (prev_lower) out.push_back('_');
                out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                prev_lower = false;
            } else {
                out.push_back(c);
                prev_lower = true;
            }
        } else {
            if (!out.empty() && out.back() != '_') out.push_back('_');
            prev_lower = false;
        }
    }
    if (!out.empty() && out.back() == '_') out.pop_back();
    return out.empty() ? "_" : out;
}

std::string open_namespaces(const std::string& pkg) {
    if (pkg.empty()) return "";
    std::ostringstream oss;
    std::stringstream ss(pkg);
    std::string segment;
    while (std::getline(ss, segment, '.')) {
        oss << "namespace " << sanitize_identifier(segment) << " { ";
    }
    return oss.str();
}
std::string close_namespaces(const std::string& pkg) {
    if (pkg.empty()) return "";
    size_t dots = std::count(pkg.begin(), pkg.end(), '.');
    std::ostringstream oss;
    for (size_t i = 0; i <= dots; ++i) oss << "} ";
    return oss.str();
}

std::string flattened_msg_name(const std::vector<std::string>& scope, const std::string& name) {
    std::ostringstream oss;
    for (size_t i = 0; i < scope.size(); ++i) {
        if (i) oss << "_";
        oss << sanitize_identifier(scope[i]);
    }
    if (!scope.empty()) oss << "_";
    oss << sanitize_identifier(name);
    return oss.str();
}

std::string label_to_string(FieldDescriptorProto::Label label) {
    using L = FieldDescriptorProto;
    switch (label) {
        case L::LABEL_REQUIRED: return "required";
        case L::LABEL_REPEATED: return "repeated";
        case L::LABEL_OPTIONAL:
        default:                return "optional";
    }
}

// NOTE: change here — message types become literal "message"
std::string type_to_string(const FieldDescriptorProto& field,
                           const google::protobuf::DescriptorPool* pool) {
    using T = FieldDescriptorProto;
    switch (field.type()) {
        case T::TYPE_DOUBLE:   return "double";
        case T::TYPE_FLOAT:    return "float";
        case T::TYPE_INT64:    return "int64";
        case T::TYPE_UINT64:   return "uint64";
        case T::TYPE_INT32:    return "int32";
        case T::TYPE_FIXED64:  return "fixed64";
        case T::TYPE_FIXED32:  return "fixed32";
        case T::TYPE_BOOL:     return "bool";
        case T::TYPE_STRING:   return "string";
        case T::TYPE_BYTES:    return "bytes";
        case T::TYPE_UINT32:   return "uint32";
        case T::TYPE_SFIXED32: return "sfixed32";
        case T::TYPE_SFIXED64: return "sfixed64";
        case T::TYPE_SINT32:   return "sint32";
        case T::TYPE_SINT64:   return "sint64";
        case T::TYPE_GROUP:    return "group";
        case T::TYPE_MESSAGE:  return "message";
        case T::TYPE_ENUM:     return "enum";
        default: return "unknown";
    }
}

std::string make_enumerator_name(const FieldDescriptorProto& f,
                                 const google::protobuf::DescriptorPool* pool) {
    const std::string label = label_to_string(f.label());
    const std::string type  = type_to_string(f, pool);
    const std::string name  = f.name();  // already lower_snake in proto
    const std::string combined = sanitize_identifier(label + "_" + type + "_" + name);
    return combined;
}

void emit_enum_for_message(std::ostringstream& out,
                           const std::string& enum_name,
                           const DescriptorProto& msg,
                           const google::protobuf::DescriptorPool* pool) {
    out << "enum class " << enum_name << " : protozero::pbf_tag_type {\n";
    for (int i = 0; i < msg.field_size(); ++i) {
        const auto& f = msg.field(i);
        out << "    " << make_enumerator_name(f, pool) << " = " << f.number();
        out << (i + 1 < msg.field_size() ? ",\n" : "\n");
    }
    out << "};\n\n";
}

void walk_messages(std::ostringstream& out,
                   const DescriptorProto& msg,
                   std::vector<std::string>& scope,
                   const google::protobuf::DescriptorPool* pool) {
    const std::string flat_name = flattened_msg_name(scope, msg.name());
    emit_enum_for_message(out, flat_name, msg, pool);

    scope.push_back(msg.name());
    for (int i = 0; i < msg.nested_type_size(); ++i) {
        if (msg.nested_type(i).options().map_entry()) continue;
        walk_messages(out, msg.nested_type(i), scope, pool);
    }
    scope.pop_back();
}

std::string generate_header(const FileDescriptorProto& file,
                            const CodeGeneratorRequest& req,
                            const google::protobuf::DescriptorPool* pool) {
    std::ostringstream out;

    out << "// Auto-generated by protoc-gen-pbftags. DO NOT EDIT.\n";
    out << "// Source: " << file.name() << "\n\n";
    out << "#pragma once\n\n";                        // <— change here
    out << "#include <protozero/types.hpp>\n";

    for (int i = 0; i < file.dependency_size(); ++i) {
        out << "#include \"" << header_name_for_proto(file.dependency(i)) << "\"\n";
    }
    if (file.dependency_size() > 0) out << "\n";

    if (!file.package().empty()) {
        out << open_namespaces(file.package()) << "\n\n";
    }

    std::vector<std::string> scope;
    for (int i = 0; i < file.message_type_size(); ++i) {
        const auto& msg = file.message_type(i);
        if (msg.options().map_entry()) continue;
        walk_messages(out, msg, scope, pool);
    }

    if (!file.package().empty()) {
        out << close_namespaces(file.package()) << "\n";
    }

    return out.str();
}

} // namespace

int main() {
    CodeGeneratorRequest request;
    if (!request.ParseFromIstream(&std::cin)) {
        std::cerr << "Failed to parse CodeGeneratorRequest.\n";
        return 1;
    }

    google::protobuf::DescriptorPool pool;
    std::unordered_map<std::string, const google::protobuf::FileDescriptor*> file_by_name;

    for (int i = 0; i < request.proto_file_size(); ++i) {
        google::protobuf::FileDescriptorProto fdp = request.proto_file(i);
        const google::protobuf::FileDescriptor* fd = pool.BuildFile(fdp);
        if (!fd) {
            std::cerr << "Failed to build FileDescriptor for " << fdp.name() << "\n";
        } else {
            file_by_name[fdp.name()] = fd;
        }
    }

    CodeGeneratorResponse response;

    for (int i = 0; i < request.file_to_generate_size(); ++i) {
        const std::string& fname = request.file_to_generate(i);

        const FileDescriptorProto* proto = nullptr;
        for (int j = 0; j < request.proto_file_size(); ++j) {
            if (request.proto_file(j).name() == fname) {
                proto = &request.proto_file(j);
                break;
            }
        }
        if (!proto) continue;

        auto* file = response.add_file();
        file->set_name(header_name_for_proto(fname));
        std::string content = generate_header(*proto, request, &pool);
        file->set_content(std::move(content));
    }

    if (!response.SerializeToOstream(&std::cout)) {
        std::cerr << "Failed to serialize CodeGeneratorResponse.\n";
        return 1;
    }
    return 0;
}