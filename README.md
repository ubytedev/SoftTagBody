# SoftTagBody.uplugin
Experimental SoftTagBody UPROPERTY metadata specifier that works with Typed Tags.

## Recommended optional plugins (used in the example below)
- TypedTagStaticImpl.uplugin

Example:
```cpp
USTRUCT()
struct FExampleCrowdActivityStrategyTagSet
{
	GENERATED_BODY()
	
	// This constructor attempts to fill all soft tag body properties with its corresponding tag value.
	FCommonActivityStrategyCrowdTagSet() { TSoftTagBodyStatics<TRemovePointer<decltype(this)>::Type>::InitFields<FCrowdActivityStrategyTag>(this); }
	
	UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true, TagDevComment = "Tag dev comment that's used when this property gets registered as native gameplay tag. "))
	FCrowdActivityStrategyTag Combat;
	
	UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true))
	FCrowdActivityStrategyTag Combat_Endurance;

	UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true))
	FCrowdActivityStrategyTag Combat_Endurance_Hardened;

	UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true))
	FCrowdActivityStrategyTag Combat_Endurance_Squishy;
};

// As long as one instance remains, the tags remain registered with the gameplay tags module.
struct FCommonActivityStrategyTagRegistration : TSharedFromThis<FCommonActivityStrategyTagRegistration>
{
	const TSoftTagBodyFieldRegistration<FExampleCrowdActivityStrategyTagSet, FCrowdActivityStrategyTag> CrowdRegistration;
};
```

```cpp
USTRUCT(Meta = (Categories = "CrowdActivityCognition"))
struct FCrowdActivityCognitionTag : public FGameplayTag
{
	GENERATED_BODY()
	END_TYPED_TAG_DECL(FCrowdActivityCognitionTag, TEXT(""))
};

USTRUCT(Meta = (Categories = "CrowdActivityStrategy"))
struct FCrowdActivityStrategyTag : public FCrowdActivityCognitionTag
{
	GENERATED_BODY()
	END_TYPED_TAG_DECL(FCrowdActivityStrategyTag, TEXT("CrowdActivityStrategy"))
};
```



Tag registration utility for FGameplayTag (derived) field members that:
- Represent a gameplay tag string. E.g: Combat_Strategy_XYZ
- Have UPROPERTY(Meta = (SoftTagBody = true)) without any Edit specifier

`TSoftTagBodyFieldRegistration` struct registers/unregisters the property's FName at construction/destruction with the GameplayTagsManager.
@note: Tag deregistration behavior is currently not available in GameplayTagsManager, as of now.

Make sure you only construct an instance:
- before `FCoreDelegates::OnPostEngineInit` or during `UGameplayTagsManager::OnLastChanceToAddNativeTags`.

If your module is responsible for creating an instance, make sure:
- The loading phase of the module is never later then `ELoadingPhase::PostDefault`.
- The module type is at least `EHostType::Runtime` or `EHostType::RuntimeAndProgram`.

After `UGameplayTagsManager::DoneAddingNativeTags` is called, we cannot add any native tags no more.
That is called during `FCoreDelegates::OnPostEngineInit`.

