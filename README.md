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
