﻿/*
* Tencent is pleased to support the open source community by making Puerts available.
* Copyright (C) 2020 THL A29 Limited, a Tencent company.  All rights reserved.
* Puerts is licensed under the BSD 3-Clause License, except for the third-party components listed in the file 'LICENSE' which may be subject to their corresponding license terms.
* This file is subject to the terms and conditions defined in file 'LICENSE', which is part of this source code package.
*/


#include "TypeScriptGeneratedClass.h"
#include "PropertyMacros.h"
#include "JSGeneratedFunction.h"

DEFINE_FUNCTION(UTypeScriptGeneratedClass::execCallJS)
{
    UFunction *Func = Stack.CurrentNativeFunction ? Stack.CurrentNativeFunction : Stack.Node;
    check(Func);
    //UE_LOG(LogTemp, Warning, TEXT("overrided function called, %s(%p)"), *Func->GetName(), Func);

    UTypeScriptGeneratedClass *Class = Cast<UTypeScriptGeneratedClass>(Func->GetOuter());
    if (Class)
    {
        auto PinedDynamicInvoker = Class->DynamicInvoker.Pin();
        if (PinedDynamicInvoker)
        {
            PinedDynamicInvoker->InvokeTsMethod(Context, Func, Stack, RESULT_PARAM);
        }
    }
}

void UTypeScriptGeneratedClass::StaticConstructor(const FObjectInitializer& ObjectInitializer)
{
    auto Class = ObjectInitializer.GetClass();

    //UE_LOG(LogTemp, Error, TEXT("UTypeScriptGeneratedClass::StaticConstructor"));
    if (auto TypeScriptGeneratedClass = Cast<UTypeScriptGeneratedClass>(Class))
    {
        TypeScriptGeneratedClass->ObjectInitialize(ObjectInitializer);
    }
    else if (auto SuperTypeScriptGeneratedClass = Cast<UTypeScriptGeneratedClass>(Class->GetSuperClass()))
    {
        SuperTypeScriptGeneratedClass->ObjectInitialize(ObjectInitializer);
    }
    else
    {
        Class->GetSuperClass()->ClassConstructor(ObjectInitializer);
    }
}

void UTypeScriptGeneratedClass::ObjectInitialize(const FObjectInitializer& ObjectInitializer)
{
    auto Object = ObjectInitializer.GetObj();
    if (auto SuperTypeScriptGeneratedClass = Cast<UTypeScriptGeneratedClass>(GetSuperClass()))
    {
        SuperTypeScriptGeneratedClass->ObjectInitialize(ObjectInitializer);
    }
    else
    {
        GetSuperClass()->ClassConstructor(ObjectInitializer);
    }

    auto PinedDynamicInvoker = DynamicInvoker.Pin();
    if (PinedDynamicInvoker)
    {
        PinedDynamicInvoker->TsConstruct(this, Object);
    }
}

void UTypeScriptGeneratedClass::RedirectToTypeScript(UFunction* InFunction)
{
    if (InFunction->Script.Num() == 0)
    {
        InFunction->Script.Add(EX_EndFunctionParms);
    }
    InFunction->FunctionFlags |= FUNC_BlueprintCallable | FUNC_BlueprintEvent | FUNC_Public | FUNC_Native;
    InFunction->SetNativeFunc(&UTypeScriptGeneratedClass::execCallJS);
    AddNativeFunction(*InFunction->GetName(), &UTypeScriptGeneratedClass::execCallJS);
}

void UTypeScriptGeneratedClass::Bind()
{
    if (HasConstructor)
    {
        //普通对象会从CDO拷贝，而CDO会从蓝图AR那反序列化（见UBlueprintGeneratedClass::SerializeDefaultObject），这会
        //导致TS的构造函数对生成的蓝图变量赋值都失效，不太符合程序员直觉，设置CPF_SkipSerialization可以跳过这个过程。
        //然而在构造对象还有一个PostConstructInit步骤，里头有个从基类的CDO拷贝值的过程（ps：UE对象构造巨复杂，对象巨大）
        //这个过程如果是CDO的话，目前只找到把属性的flag设置为CPF_Transient | CPF_InstancedReference才能搞定
        //TODO: 后续尝试下TypeScript生成类不继承UBlueprintGeneratedClass的实现，能实现的话优雅些
        for (TFieldIterator<PropertyMacro> PropertyIt(this, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt)
        {
            PropertyMacro *Property = *PropertyIt;
            Property->SetPropertyFlags(CPF_SkipSerialization | CPF_Transient | CPF_InstancedReference);
        }

        //可避免非CDO的在PostConstructInit从基类拷贝值
        //ClassFlags |= CLASS_Native;
    }

    for (TFieldIterator<UFunction> FuncIt(this, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
    {
        auto Function = *FuncIt;
        RedirectToTypeScript(Function);
    }

    Super::Bind();
    ReBind = true;
}