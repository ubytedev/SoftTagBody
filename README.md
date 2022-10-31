# SoftTagBody.uplugin (Experimental)
Experimental SoftTagBody UPROPERTY metadata specifier that works with Typed gameplay tags.

For an example plugin, view [SoftTagBody-Example](https://github.com/ubytedev/SoftTagBody-Example).

## Example:
```cpp
/** Since this is CDO-based, use GetDefault<FExampleTagSet>() to read fields */
USTRUCT()
struct FExampleTagSet
{
    GENERATED_BODY()
    
    // This constructor attempts to fill all soft tag body properties with its corresponding tag value.
    FExampleTagSet() { TSoftTagBodyStatics<TRemovePointer<decltype(this)>::Type>::InitFields<FAgentGameplayEventTag, FAgentTag>(this); }
    
    UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true, TagDevComment = "Tag dev comment that's used when this property gets registered as native gameplay tag. "))
    FAgentGameplayEventTag Combat; // Agent.GameplayEvent.Combat
    
    UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true))
    FAgentGameplayEventTag Combat_Endurance; // Agent.GameplayEvent.Combat.Endurance

    UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true))
    FAgentGameplayEventTag Combat_Endurance_Hardened; // Agent.GameplayEvent.Combat.Hardened

    UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true))
    FAgentGameplayEventTag Combat_Endurance_Squishy; // Agent.GameplayEvent.Combat.Squishy
	
    UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true))
    FAgentTag Mobility_Fast; // Agent.Mobility.Fast
	
    UPROPERTY(VisibleAnywhere, Meta = (SoftTagBody = true))
    FAgentTag Mobility_Slow; // Agent.Mobility.Slow
};

// As long as one instance remains, the tags remain registered with the gameplay tags module.
struct FExampleTagRegistration : TSharedFromThis<FExampleTagRegistration>
{
    const TSoftTagBodyFieldRegistration<FExampleTagSet, FAgentGameplayEventTag> Registration1;
    const TSoftTagBodyFieldRegistration<FExampleTagSet, FAgentTag> Registration2;
};

// Using the following typed gameplay tags inheritance hierarchy:

// The Categories metadata specifier is only responsible for filtering the GameplayTag in the editor.
USTRUCT(Meta = (Categories = "Agent"))
struct FAgentTag : public FGameplayTag
{
    GENERATED_BODY()
    END_TYPED_TAG_DECL(FAgentTag, TEXT("Agent"))
};

// The Categories metadata specifier is only responsible for filtering the GameplayTag in the editor.
USTRUCT(Meta = (Categories = "Agent.GameplayEvent"))
struct FAgentGameplayEventTag : public FAgentTag
{
    GENERATED_BODY()
    END_TYPED_TAG_DECL(FAgentGameplayEventTag, TEXT("Agent.GameplayEvent"))
};

```
## Recommended optional plugins (used in the example above)
| Plugin | Reason |
| :----- | :----- |
| [TypedTagStaticImpl](https://github.com/ubytedev/TypedTagStaticImpl) | Introduces the `END_TYPED_TAG_DECL` macro, which is used to declare typed gameplay tags. |

# Long-term vision

Full UHT reflection instead of manually typing the initialization out in the constructor.
