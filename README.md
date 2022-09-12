# SoftTagBody.uplugin (Experimental)
Experimental SoftTagBody UPROPERTY metadata specifier that works with Typed gameplay tags.

## Example:
```cpp
USTRUCT()
struct FExampleTagSet
{
	GENERATED_BODY()
	
	// This constructor attempts to fill all soft tag body properties with its corresponding tag value.
	FExampleTagSet() { TSoftTagBodyStatics<TRemovePointer<decltype(this)>::Type>::InitFields<FAgentGameplayEventTag>(this); }
	
	UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true, TagDevComment = "Tag dev comment that's used when this property gets registered as native gameplay tag. "))
	FAgentGameplayEventTag Combat; // Agent.GameplayEvent.Combat
	
	UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true))
	FAgentGameplayEventTag Combat_Endurance; // Agent.GameplayEvent.Combat.Endurance

	UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true))
	FAgentGameplayEventTag Combat_Endurance_Hardened; // Agent.GameplayEvent.Combat.Hardened

	UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true))
	FAgentGameplayEventTag Combat_Endurance_Squishy; // Agent.GameplayEvent.Combat.Squishy
};

// As long as one instance remains, the tags remain registered with the gameplay tags module.
struct FCommonActivityStrategyTagRegistration : TSharedFromThis<FCommonActivityStrategyTagRegistration>
{
	const TSoftTagBodyFieldRegistration<FExampleTagSet, FAgentGameplayEventTag> Registration;
};
```

Using the following typed gameplay tags inheritance hierarchy:

```cpp
USTRUCT()
struct FAgentTag : public FGameplayTag
{
	GENERATED_BODY()
	END_TYPED_TAG_DECL(FAgentTag, TEXT("Agent"))
};

USTRUCT()
struct FAgentGameplayEventTag : public FAgentTag
{
	GENERATED_BODY()
	END_TYPED_TAG_DECL(FAgentGameplayEventTag, TEXT("GameplayEvent"))
};
```
## Recommended optional plugins (used in the example above)
| Plugin | Reason |
| :----- | :----- |
| TypedTagStaticImpl.uplugin | Introduces the `END_TYPED_TAG_DECL` macro, which is used to declare typed gameplay tags. |

# About the plugin

## `TSoftTagBodyFieldRegistration`
Tag registration utility for FGameplayTag (derived) field members that:
- Represent a gameplay tag string. E.g: Combat_Strategy_XYZ
- Have UPROPERTY(Meta = (SoftTagBody = true)) without any Edit specifier

This struct registers/unregisters the property's FName at construction/destruction with the GameplayTagsManager.

@note: Tag deregistration behavior is currently not available in GameplayTagsManager, as of now.

Make sure you only construct an instance:
- before `FCoreDelegates::OnPostEngineInit` or during `UGameplayTagsManager::OnLastChanceToAddNativeTags`.

If your module is responsible for creating an instance, make sure:
- The loading phase of the module is never later then `ELoadingPhase::PostDefault`.
- The module type is at least `EHostType::Runtime` or `EHostType::RuntimeAndProgram`.

After `UGameplayTagsManager::DoneAddingNativeTags` is called, we cannot add any native tags no more.
That is called during `FCoreDelegates::OnPostEngineInit`.

## `TSoftTagBodyStatics`

Find all FGameplayTag (derived) field members that have UPROPERTY(Meta = (SoftTagBody = true)) of a struct instance,
and initialize them with the tag it itself presumably represent.
@note Since the property should be uneditable in the editor, properties with any Edit specifier are not supported.
If the tag was not registered with the GameplayTagsManager, the field will simply be left uninitialized.
