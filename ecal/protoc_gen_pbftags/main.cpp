/* ========================= eCAL LICENSE =================================
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
 * ========================= eCAL LICENSE =================================
*/

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>

#include <algorithm>
#include <cctype>
#include <functional>
#include <memory>
#include <sstream>
#include <string>

using google::protobuf::Descriptor;
using google::protobuf::EnumDescriptor;
using google::protobuf::FileDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::compiler::CodeGenerator;
using google::protobuf::compiler::GeneratorContext;

namespace {

  std::string header_name_for_proto(const std::string& proto_name) {
    std::string out = proto_name;
    if (out.size() >= 6 && out.substr(out.size() - 6) == ".proto")
      out.replace(out.size() - 6, 6, ".pbftags.h");
    else
      out += ".pbftags.h";
    return out;
  }

  std::string sanitize_identifier(std::string id) {
    for (char& c : id) {
      if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_')) c = '_';
    }
    if (!id.empty() && std::isdigit(static_cast<unsigned char>(id[0]))) id.insert(id.begin(), '_');
    return id;
  }

  std::string to_snake(std::string s) {
    std::string out; out.reserve(s.size() * 2);
    bool prev_lower = false;
    for (char c : s) {
      if (std::isalnum(static_cast<unsigned char>(c))) {
        if (std::isupper(static_cast<unsigned char>(c))) {
          if (prev_lower) out.push_back('_');
          out.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
          prev_lower = false;
        }
        else {
          out.push_back(c);
          prev_lower = true;
        }
      }
      else {
        if (!out.empty() && out.back() != '_') out.push_back('_');
        prev_lower = false;
      }
    }
    if (!out.empty() && out.back() == '_') out.pop_back();
    if (out.empty()) out = "_";
    return out;
  }

  std::string label_to_string(FieldDescriptor::Label label) {
    switch (label) {
    case FieldDescriptor::LABEL_REQUIRED: return "required";
    case FieldDescriptor::LABEL_REPEATED: return "repeated";
    case FieldDescriptor::LABEL_OPTIONAL:
    default: return "optional";
    }
  }

  std::string field_kind(const FieldDescriptor* f) {
    switch (f->type()) {
    case FieldDescriptor::TYPE_DOUBLE:   return "double";
    case FieldDescriptor::TYPE_FLOAT:    return "float";
    case FieldDescriptor::TYPE_INT64:    return "int64";
    case FieldDescriptor::TYPE_UINT64:   return "uint64";
    case FieldDescriptor::TYPE_INT32:    return "int32";
    case FieldDescriptor::TYPE_FIXED64:  return "fixed64";
    case FieldDescriptor::TYPE_FIXED32:  return "fixed32";
    case FieldDescriptor::TYPE_BOOL:     return "bool";
    case FieldDescriptor::TYPE_STRING:   return "string";
    case FieldDescriptor::TYPE_BYTES:    return "bytes";
    case FieldDescriptor::TYPE_UINT32:   return "uint32";
    case FieldDescriptor::TYPE_SFIXED32: return "sfixed32";
    case FieldDescriptor::TYPE_SFIXED64: return "sfixed64";
    case FieldDescriptor::TYPE_SINT32:   return "sint32";
    case FieldDescriptor::TYPE_SINT64:   return "sint64";
    case FieldDescriptor::TYPE_GROUP:    return "group";
    case FieldDescriptor::TYPE_MESSAGE:  return "message"; // literal
    case FieldDescriptor::TYPE_ENUM:     return "enum";
    default: return "unknown";
    }
  }

  std::string enumerator_name(const FieldDescriptor* f) {
    std::string label = label_to_string(f->label());
    std::string type = field_kind(f);
    std::string name = f->name(); // already lower_snake in proto
    return sanitize_identifier(label + "_" + type + "_" + name);
  }

  // Flatten nested message path: Outer_Inner_Leaf
  std::string flattened_msg(const Descriptor* d) {
    std::string s;
    for (const auto* cur = d; cur != nullptr; cur = cur->containing_type()) {
      std::string part = sanitize_identifier(cur->name());
      s = s.empty() ? part : (part + "_" + s);
    }
    return s;
  }

  // Flatten nested enum path: Outer_Inner_EnumName
  std::string flattened_enum(const EnumDescriptor* e) {
    std::string s = sanitize_identifier(e->name());
    const Descriptor* parent = e->containing_type();
    while (parent) {
      s = sanitize_identifier(parent->name()) + "_" + s;
      parent = parent->containing_type();
    }
    return s;
  }

  void open_namespaces(std::ostringstream& os, const std::string& pkg) {
    if (pkg.empty()) return;
    size_t start = 0;
    while (true) {
      size_t dot = pkg.find('.', start);
      std::string seg = sanitize_identifier(pkg.substr(start, dot - start));
      if (!seg.empty()) os << "namespace " << seg << " { ";
      if (dot == std::string::npos) break;
      start = dot + 1;
    }
    os << "\n";
  }

  void close_namespaces(std::ostringstream& os, const std::string& pkg) {
    if (pkg.empty()) return;
    size_t levels = 1 + std::count(pkg.begin(), pkg.end(), '.');
    for (size_t i = 0; i < levels; ++i) os << "} ";
    os << "\n";
  }

  // --- emission ---

  void emit_enum_for_message(std::ostringstream& os, const Descriptor* msg) {
    const std::string name = flattened_msg(msg);

    os << "enum class " << name << " : ::protozero::pbf_tag_type {\n";
    for (int i = 0; i < msg->field_count(); ++i) {
      const auto* f = msg->field(i);
      os << "    " << enumerator_name(f) << " = " << f->number();
      os << (i + 1 < msg->field_count() ? ",\n" : "\n");
    }
    os << "};\n\n";
    os << "inline constexpr uint32_t operator+(" << name << " e) {\n";
    os << "    return static_cast<uint32_t>(e);\n";
    os << "}\n\n";
  }

  void emit_proto_enum(std::ostringstream& os, const EnumDescriptor* e) {
    const std::string name = flattened_enum(e);
    os << "enum class " << name << " : std::int32_t {\n";
    for (int i = 0; i < e->value_count(); ++i) {
      const auto* v = e->value(i);
      os << "    " << sanitize_identifier(v->name()) << " = " << v->number();
      os << (i + 1 < e->value_count() ? ",\n" : "\n");
    }
    os << "};\n\n";
    os << "inline constexpr std::int32_t operator+(" << name << " v) {\n";
    os << "    return static_cast<std::int32_t>(v);\n";
    os << "}\n\n";
  }

  void walk_messages(std::ostringstream& os, const FileDescriptor* file) {
    // Top-level messages
    for (int i = 0; i < file->message_type_count(); ++i) {
      const Descriptor* d = file->message_type(i);
      if (d->options().map_entry()) continue;

      emit_enum_for_message(os, d);

      // nested recurse
      std::function<void(const Descriptor*)> rec;
      rec = [&](const Descriptor* m) {
        // nested messages
        for (int j = 0; j < m->nested_type_count(); ++j) {
          const auto* n = m->nested_type(j);
          if (n->options().map_entry()) continue;
          emit_enum_for_message(os, n);
          rec(n);
        }
        };
      // include enums directly nested in this top-level message
      for (int k = 0; k < d->enum_type_count(); ++k) {
        emit_proto_enum(os, d->enum_type(k));
      }
      rec(d);
    }

    // Top-level enums (not nested in any message)
    for (int i = 0; i < file->enum_type_count(); ++i) {
      emit_proto_enum(os, file->enum_type(i));
    }
  }

} // namespace

class PbftagsGenerator final : public CodeGenerator {
public:
  bool Generate(const FileDescriptor* file,
    const std::string& /*parameter*/,
    GeneratorContext* context,
    std::string* /*error*/) const override {
    const std::string out_name = header_name_for_proto(file->name());
    std::unique_ptr<google::protobuf::io::ZeroCopyOutputStream> output(context->Open(out_name));
    google::protobuf::io::Printer p(output.get(), '$');

    std::ostringstream body;
    body << "// Auto-generated by protoc-gen-pbftags. DO NOT EDIT.\n";
    body << "// Source: " << file->name() << "\n\n";
    body << "#pragma once\n\n";
    body << "#include <protozero/types.hpp>\n";
    body << "#include <cstdint>\n";
    body << "\n";

    if (!file->package().empty()) open_namespaces(body, file->package());

    walk_messages(body, file);

    if (!file->package().empty()) close_namespaces(body, file->package());

    p.PrintRaw(body.str());
    return true;
  }
};

// Standard plugin entry point
int main(int argc, char* argv[]) {
  PbftagsGenerator gen;
  return google::protobuf::compiler::PluginMain(argc, argv, &gen);
}