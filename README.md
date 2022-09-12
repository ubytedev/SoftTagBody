# Soft Tag Body
Experimental SoftTagBody UPROPERTY metadata specifier

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

