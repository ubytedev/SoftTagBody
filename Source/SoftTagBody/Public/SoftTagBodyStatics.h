// Copyright ubyte digital. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagsManager.h"

namespace SoftTagBodyPrivate
{
	// The current requirements for native gameplay tag types, since they do not necessarily share the same base class 
	template <typename FGameplayTagType>
	struct TIsNativeTag
	{
		static constexpr bool Value =
			TIsSame<decltype(&FGameplayTagType::AddNativeTag), FGameplayTagType(*)(const FString&, const FString&)>::Value &&
			TIsSame<decltype(&FGameplayTagType::GetRootTag),   FGameplayTagType(*)()>::Value;
	};
	
	template <typename FGameplayTagType = FGameplayTag>
	static FGameplayTag AddNativeTag(FString TagBody, const FString& TagDevComment = FString())
	{
		if constexpr (TIsNativeTag<FGameplayTagType>::Value)
		{
			if (FGameplayTagType::GetRootTag().IsValid())
			{
				// Remove the unnecessary prefix if present
				const FString PotentialPrefix = FGameplayTagType::GetRootTag().ToString() + TEXT(".");
				TagBody.RemoveFromStart(PotentialPrefix);
			}
			
			return FGameplayTagType::AddNativeTag(TagBody, TagDevComment);
		}
		else
		{
			return UGameplayTagsManager::Get().AddNativeGameplayTag(FName(TagBody), TagDevComment); 
		}
	}

	template <typename FGameplayTagType = FGameplayTag>
	static FString BuildTagBodyFromProperty(FProperty* Property)
	{
		const FString PropertyBody = Property->GetName().Replace(TEXT("_"), TEXT("."));
		if constexpr (TIsNativeTag<FGameplayTagType>::Value)
		{
			if (FGameplayTagType::GetRootTag().IsValid())
			{
				const FString NativeRootTag = FGameplayTagType::GetRootTag().ToString();
				if (PropertyBody.StartsWith(NativeRootTag))
				{
					return PropertyBody;
				}

				return NativeRootTag + TEXT(".") + PropertyBody; 
			}
		}
		
		return PropertyBody;
	}
}

/**
 * Tag registration utility for FGameplayTag (derived) field members that:
 * - Represent a gameplay tag string. E.g: Combat_Strategy_XYZ
 * - Have UPROPERTY(Meta = (SoftTagBody = true)) without any Edit specifier
 *
 * This struct registers/unregisters the property's FName at construction/destruction with the GameplayTagsManager.
 * @note: Tag deregistration behavior is currently not available in GameplayTagsManager, as of now.
 * 
 * Make sure you only construct an instance:
 * - before FCoreDelegates::OnPostEngineInit or during UGameplayTagsManager::OnLastChanceToAddNativeTags.
 *
 * If your module is responsible for creating an instance, make sure:
 * - The loading phase of the module is never later then ELoadingPhase::PostDefault.
 * - The module type is at least EHostType::Runtime or EHostType::RuntimeAndProgram.
 * 
 * After UGameplayTagsManager::DoneAddingNativeTags is called, we cannot add any native tags no more.
 * That is called during FCoreDelegates::OnPostEngineInit
 */
template <typename FStructType, typename... FGameplayTagTypes>
class TSoftTagBodyFieldRegistration
{	
public:
	TSoftTagBodyFieldRegistration()
	{
		// The following guard should be here, but it's inaccessible unless we modify the engine. Follow the documentation of this structure.
		//	if (!UGameplayTagsManager::Get().bDoneAddingNativeTags)
		(RegisterGameplayTagType<FGameplayTagTypes>(), ...);
	}
	
	~TSoftTagBodyFieldRegistration()
	{
		(UnregisterGameplayTagType<FGameplayTagTypes>(), ...);
	}

private:
	template <typename FGameplayTagType = FGameplayTag>
	static void RegisterGameplayTagType()
	{
		static_assert(TIsDerivedFrom<FGameplayTagType, FGameplayTag>::Value, "Template argument must at least be an FGameplayTag");
		
		for (FStructProperty* Property : TFieldRange<FStructProperty>(FStructType::StaticStruct(), EFieldIterationFlags::None))
		{
			if (!Property->Struct->IsChildOf(FGameplayTagType::StaticStruct())
				|| !Property->GetBoolMetaData(TEXT("SoftTagBody"))
				|| !Property->HasAnyPropertyFlags(CPF_EditConst))
			{
				continue;
			}
			
			FString TagBody = SoftTagBodyPrivate::BuildTagBodyFromProperty<FGameplayTagType>(Property);
			if (const FString* TagDevComment = Property->FindMetaData(TEXT("TagDevComment")))
			{
				SoftTagBodyPrivate::AddNativeTag<FGameplayTagType>(TagBody, *TagDevComment);
			}
			else
			{
				SoftTagBodyPrivate::AddNativeTag<FGameplayTagType>(TagBody);
			}
		}
	}

	template <class FGameplayTagType = FGameplayTag>
	static void UnregisterGameplayTagType()
	{
		static_assert(TIsDerivedFrom<FGameplayTagType, FGameplayTag>::Value, "Template argument must at least be an FGameplayTag");
		
		// UGameplayTagsManager has no RemoveNativeTag behavior (yet). Even FNativeGameplayTag says it's not available.
	}
};

template <class FStructType>
struct TSoftTagBodyStatics
{
	/**
	 * Find all FGameplayTag (derived) field members that have UPROPERTY(Meta = (SoftTagBody = true)) of a struct instance,
	 * and initialize them with the tag it itself presumably represent.
	 * @note Since the property should be uneditable in the editor, properties with any Edit specifier are not supported.
	 *
	 * If the tag was not registered with the GameplayTagsManager, the field will simply be left uninitialized.
	 */
	template <typename... FGameplayTagTypes>
	static void InitFields(FStructType* Struct, bool bErrorIfNotInitialized = false)
	{
		(InitGameplayTagType<FGameplayTagTypes>(Struct, bErrorIfNotInitialized), ...);
	}

	template <typename... FGameplayTagTypes>
	static void DeinitFields(FStructType* Struct)
	{
		(DeinitGameplayTagType<FGameplayTagTypes>(Struct), ...);
	}
	
	template <typename FGameplayTagType>
	static TArray<FGameplayTagType> FindFields(FStructType* Struct)
	{
		return FindGameplayTagTypeFields<FGameplayTagType>(Struct);
	}

private:
	template <class FGameplayTagType = FGameplayTag>
	static void InitGameplayTagType(FStructType* Struct, bool bErrorIfNotInitialized)
	{
		static_assert(TIsDerivedFrom<FGameplayTagType, FGameplayTag>::Value, "Template argument must at least be an FGameplayTag");
	
		for (FStructProperty* Property : TFieldRange<FStructProperty>(FStructType::StaticStruct(), EFieldIterationFlags::None))
		{
			if (!Property->Struct->IsChildOf(FGameplayTagType::StaticStruct())
				 || !Property->GetBoolMetaData(TEXT("SoftTagBody"))
				 || !Property->HasAnyPropertyFlags(CPF_EditConst))
			{
				continue;
			}
			
			FString TagBody = SoftTagBodyPrivate::BuildTagBodyFromProperty<FGameplayTagType>(Property);
			FGameplayTag RequestedGameplayTag = UGameplayTagsManager::Get().RequestGameplayTag(FName(TagBody), bErrorIfNotInitialized);
			
			if (RequestedGameplayTag.IsValid())
			{
				FGameplayTagType* TypedTag = Property->ContainerPtrToValuePtr<FGameplayTagType>(Struct);
				*TypedTag = RequestedGameplayTag;
			}
		}
	}

	template <class FGameplayTagType = FGameplayTag>
	static void DeinitGameplayTagType(FStructType* Struct)
	{
		static_assert(TIsDerivedFrom<FGameplayTagType, FGameplayTag>::Value, "Template argument must at least be an FGameplayTag");
		
		for (FStructProperty* Property : TFieldRange<FStructProperty>(FStructType::StaticStruct(), EFieldIterationFlags::None))
		{
			if (!Property->Struct->IsChildOf(FGameplayTagType::StaticStruct())
				 || !Property->GetBoolMetaData(TEXT("SoftTagBody"))
				 || !Property->HasAnyPropertyFlags(CPF_EditConst))
			{
				continue;
			}
			
			FGameplayTagType* TypedTag = Property->ContainerPtrToValuePtr<FGameplayTagType>(Struct);
			*TypedTag = FGameplayTagType();
		}
	}

	template <typename FGameplayTagType = FGameplayTag>
	static TArray<FGameplayTagType> FindGameplayTagTypeFields(FStructType* Struct)
	{
		static_assert(TIsDerivedFrom<FGameplayTagType, FGameplayTag>::Value, "Template argument must at least be an FGameplayTag");

		TArray<FGameplayTagType> Fields;
		for (FStructProperty* Property : TFieldRange<FStructProperty>(FStructType::StaticStruct(), EFieldIterationFlags::None))
		{
			if (!Property->Struct->IsChildOf(FGameplayTagType::StaticStruct())
				 || !Property->GetBoolMetaData(TEXT("SoftTagBody"))
				 || !Property->HasAnyPropertyFlags(CPF_EditConst))
			{
				continue;
			}

			Fields.Add(Property->ContainerPtrToValuePtr<FGameplayTagType>(Struct));
		}

		return Fields;
	}
};



