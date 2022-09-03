/**
 * src/sequence_diagram/visitor/translation_unit_visitor.h
 *
 * Copyright (c) 2021-2022 Bartek Kryza <bkryza@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include "config/config.h"
#include "sequence_diagram/model/diagram.h"

#include <clang/AST/Expr.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/SourceManager.h>

namespace clanguml::sequence_diagram::visitor {

class translation_unit_visitor
    : public clang::RecursiveASTVisitor<translation_unit_visitor> {
public:
    translation_unit_visitor(clang::SourceManager &sm,
        clanguml::sequence_diagram::model::diagram &diagram,
        const clanguml::config::sequence_diagram &config);

    virtual bool VisitCallExpr(clang::CallExpr *expr);

    virtual bool VisitCXXMethodDecl(clang::CXXMethodDecl *method);

    virtual bool VisitCXXRecordDecl(clang::CXXRecordDecl *cls);

    virtual bool VisitFunctionDecl(clang::FunctionDecl *function_declaration);

    clanguml::sequence_diagram::model::diagram &diagram();

    const clanguml::config::sequence_diagram &config() const;

    void finalize() { }

private:
    clang::SourceManager &source_manager_;

    // Reference to the output diagram model
    clanguml::sequence_diagram::model::diagram &diagram_;

    // Reference to class diagram config
    const clanguml::config::sequence_diagram &config_;

    clang::CXXRecordDecl *current_class_decl_;
    clang::CXXMethodDecl *current_method_decl_;
    clang::FunctionDecl *current_function_decl_;
};

}
