The String Table Generator is a tool used to generate string table entries based on the content of a DataTable. You can adjust generation settings to adapt it to the way you want to generate the string table and what data should be used for the generation.

How to use it?
The String Table Generation can now be found in the toolbar of the DataTable editor. So you can simply open the DataTable containing the data you want to save into a String Table and click on this button in the toolbar.

A new window is opening, this is the settings window in which you can configure how you want the String Table to be generated.

Choosing a String Table
The first thing you need to specify is in which String Table you want to store the generated entries. You have 2 ways to do that, either by creating a new String Table or using an existing one:

Selecting which texts are affected
Then you can choose which text you want to save into the String Table. You can choose to save all the texts from the DataTable or only one type of text for each row of the DataTable.

If you check Save All Properties, it's going to find all the texts inside the DataTable and generate a String Table entry for them. For example if we are in the DataTable InGameAdvisor it's going to save these 4 texts into the String Table for each row of the DataTable. So in this case 8 String Table entries, 4 for the police officer, 4 for the tech operator.

If Save All Properties is not checked, we have to choose which text we want to save into the String table. The option Property Name is a dropdown listing all the texts that you can use for the generation. Select the one that you want to use.

It's going to generate a String Table key for this text for every row of the DataTable. So with the same example, if we select Name in the dropdown it's going to generate 2 String Table entries. 1 for the police officer's name and 1 for the tech operator's name.

The next option is Copy Already Localized Keys, if this box is not checked it's going to skip the texts that are already linked to a String Table. If checked, it's going to copy the String Table entry from the old String Table to the new one and change the link in the Text to the new location.

String Table Keys
As we are generating String Table entries we have a to determine what the generated keys should be for every time we make a new one. The following formatting is used to make new keys:

Prefix_RowName_PropertyName_Suffix
The Prefix and suffix can be specified into this window. One or the other has to be filled in order to run the generation.

A number will sometimes be added after the PropertyName if there's multiple keys that would end up with the same keys.

Generate
Once you're happy with your settings, hit the generate button. A popup will show up if your settings are invalid with the details of what is wrong.

If everything is correct the generation will begin and the modified String Table will show up with the new entries once the generation is completed.
