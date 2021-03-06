/************************************************************************
*
* CppSharp
* Licensed under the simplified BSD license. All rights reserved.
*
************************************************************************/

#include "Parser.h"

#include <clang/AST/ASTContext.h>

using namespace CppSharp::CppParser;

//-----------------------------------//

static RawCommentKind
ConvertRawCommentKind(clang::RawComment::CommentKind Kind)
{
    using clang::RawComment;

    switch(Kind)
    {
    case RawComment::RCK_Invalid: return RawCommentKind::Invalid;
    case RawComment::RCK_OrdinaryBCPL: return RawCommentKind::OrdinaryBCPL;
    case RawComment::RCK_OrdinaryC: return RawCommentKind::OrdinaryC;
    case RawComment::RCK_BCPLSlash: return RawCommentKind::BCPLSlash;
    case RawComment::RCK_BCPLExcl: return RawCommentKind::BCPLExcl;
    case RawComment::RCK_JavaDoc: return RawCommentKind::JavaDoc;
    case RawComment::RCK_Qt: return RawCommentKind::Qt;
    case RawComment::RCK_Merged: return RawCommentKind::Merged;
    }

    llvm_unreachable("Unknown comment kind");
}

RawComment* Parser::WalkRawComment(const clang::RawComment* RC)
{
    using namespace clang;

    auto &SM = C->getSourceManager();
    auto Comment = new RawComment();
    Comment->RawCommentKind = ConvertRawCommentKind(RC->getKind());
    Comment->Text = RC->getRawText(SM);
    Comment->BriefText = RC->getBriefText(*AST);

    return Comment;
}

#if 0
static InlineCommandComment::RenderKind
ConvertRenderKind(clang::comments::InlineCommandComment::RenderKind Kind)
{
    using namespace clang::comments;
    switch(Kind)
    {
    case InlineCommandComment::RenderNormal:
        return InlineCommandComment::RenderKind::RenderNormal;
    case InlineCommandComment::RenderBold:
        return InlineCommandComment::RenderKind::RenderBold;
    case InlineCommandComment::RenderMonospaced:
        return InlineCommandComment::RenderKind::RenderMonospaced;
    case InlineCommandComment::RenderEmphasized:
        return InlineCommandComment::RenderKind::RenderEmphasized;
    }
    llvm_unreachable("Unknown render kind");
}

static ParamCommandComment::PassDirection
ConvertParamPassDirection(clang::comments::ParamCommandComment::PassDirection Dir)
{
    using namespace clang::comments;
    switch(Dir)
    {
    case ParamCommandComment::In:
        return ParamCommandComment::PassDirection::In;
    case ParamCommandComment::Out:
        return ParamCommandComment::PassDirection::Out;
    case ParamCommandComment::InOut:
        return ParamCommandComment::PassDirection::InOut;
    }
    llvm_unreachable("Unknown parameter pass direction");
}

static void HandleBlockCommand(const clang::comments::BlockCommandComment *CK,
                               BlockCommandComment^ BC)
{
    using namespace clix;

    BC->CommandId = CK->getCommandID();
    for (unsigned I = 0, E = CK->getNumArgs(); I != E; ++I)
    {
        auto Arg = BlockCommandComment::Argument();
        Arg.Text = marshalString<E_UTF8>(CK->getArgText(I));
        BC->Arguments->Add(Arg);
    }
}
#endif

static Comment* ConvertCommentBlock(clang::comments::Comment* C)
{
    using namespace clang;
    using clang::comments::Comment;

    // This needs to have an underscore else we get an ICE under VS2012.
    CppSharp::CppParser::AST::Comment* _Comment = 0;

    switch(C->getCommentKind())
    {
    case Comment::FullCommentKind:
    {
        auto CK = cast<clang::comments::FullComment>(C);
        auto FC = new FullComment();
        _Comment = FC;
#if 0
        for (auto I = CK->child_begin(), E = CK->child_end(); I != E; ++I)
        {
            auto Content = ConvertCommentBlock(*I);
            FC->Blocks->Add(dynamic_cast<BlockContentComment^>(Content));
        }
        break;
#endif
    }
#if 0
    case Comment::BlockCommandCommentKind:
    {
        auto CK = cast<const clang::comments::BlockCommandComment>(C);
        auto BC = new BlockCommandComment();
        _Comment = BC;
        HandleBlockCommand(CK, BC);
        break;
    }
    case Comment::ParamCommandCommentKind:
    {
        auto CK = cast<clang::comments::ParamCommandComment>(C);
        auto PC = new ParamCommandComment();
        _Comment = PC;
        HandleBlockCommand(CK, PC);
        PC->Direction = ConvertParamPassDirection(CK->getDirection());
        if (CK->isParamIndexValid() && !CK->isVarArgParam())
            PC->ParamIndex = CK->getParamIndex();
        break;
    }
    case Comment::TParamCommandCommentKind:
    {
        auto CK = cast<clang::comments::TParamCommandComment>(C);
        _Comment = new TParamCommandComment();
        auto TC = new TParamCommandComment();
        _Comment = TC;
        HandleBlockCommand(CK, TC);
        if (CK->isPositionValid())
            for (unsigned I = 0, E = CK->getDepth(); I != E; ++I)
                TC->Position->Add(CK->getIndex(I));
        break;
    }
    case Comment::VerbatimBlockCommentKind:
    {
        auto CK = cast<clang::comments::VerbatimBlockComment>(C);
        auto VB = new VerbatimBlockComment();
        _Comment = VB;
        for (auto I = CK->child_begin(), E = CK->child_end(); I != E; ++I)
        {
            auto Line = ConvertCommentBlock(*I);
            VB->Lines->Add(dynamic_cast<VerbatimBlockLineComment^>(Line));
        }
        break;
    }
    case Comment::VerbatimLineCommentKind:
    {
        auto CK = cast<clang::comments::VerbatimLineComment>(C);
        auto VL = new VerbatimLineComment();
        _Comment = VL;
        VL->Text = marshalString<E_UTF8>(CK->getText());
        break;
    }
    case Comment::ParagraphCommentKind:
    {
        auto CK = cast<clang::comments::ParagraphComment>(C);
        auto PC = new ParagraphComment();
        _Comment = PC;
        for (auto I = CK->child_begin(), E = CK->child_end(); I != E; ++I)
        {
            auto Content = ConvertCommentBlock(*I);
            PC->Content->Add(dynamic_cast<InlineContentComment^>(Content));
        }
        PC->IsWhitespace = CK->isWhitespace();
        break;
    }
    case Comment::HTMLStartTagCommentKind:
    {
        auto CK = cast<clang::comments::HTMLStartTagComment>(C);
        auto TC = new HTMLStartTagComment();
        _Comment = TC;
        TC->TagName = marshalString<E_UTF8>(CK->getTagName());
        for (unsigned I = 0, E = CK->getNumAttrs(); I != E; ++I)
        {
            auto A = CK->getAttr(I);
            auto Attr = HTMLStartTagComment::Attribute();
            Attr.Name = marshalString<E_UTF8>(A.Name);
            Attr.Value = marshalString<E_UTF8>(A.Value);
            TC->Attributes->Add(Attr);
        }
        break;
    }
    case Comment::HTMLEndTagCommentKind:
    {
        auto CK = cast<clang::comments::HTMLEndTagComment>(C);
        auto TC = new HTMLEndTagComment();
        _Comment = TC;
        TC->TagName = marshalString<E_UTF8>(CK->getTagName());
        break;
    }
    case Comment::TextCommentKind:
    {
        auto CK = cast<clang::comments::TextComment>(C);
        auto TC = new TextComment();
        _Comment = TC;
        TC->Text = marshalString<E_UTF8>(CK->getText());
        break;
    }
    case Comment::InlineCommandCommentKind:
    {
        auto CK = cast<clang::comments::InlineCommandComment>(C);
        auto IC = new InlineCommandComment();
        _Comment = IC;
        IC->Kind = ConvertRenderKind(CK->getRenderKind());
        for (unsigned I = 0, E = CK->getNumArgs(); I != E; ++I)
        {
            auto Arg = InlineCommandComment::Argument();
            Arg.Text = marshalString<E_UTF8>(CK->getArgText(I));
            IC->Arguments->Add(Arg);
        }       
        break;
    }
    case Comment::VerbatimBlockLineCommentKind:
    {
        auto CK = cast<clang::comments::VerbatimBlockLineComment>(C);
        auto VL = new VerbatimBlockLineComment();
        _Comment = VL;
        VL->Text = marshalString<E_UTF8>(CK->getText());
        break;
    }
#endif
    case Comment::NoCommentKind: return nullptr;
    default:
        llvm_unreachable("Unknown comment kind");
    }

    assert(_Comment && "Invalid comment instance");
    return _Comment;
}

void Parser::HandleComments(clang::Decl* D, Declaration* Decl)
{
    using namespace clang;

    const clang::RawComment* RC = 0;
    if (!(RC = AST->getRawCommentForAnyRedecl(D)))
        return;

    auto RawComment = WalkRawComment(RC);
    Decl->Comment = RawComment;

    if (clang::comments::FullComment* FC = RC->parse(*AST, &C->getPreprocessor(), D))
    {
        auto CB = static_cast<FullComment*>(ConvertCommentBlock(FC));
        RawComment->FullComment = CB;
    }
}
