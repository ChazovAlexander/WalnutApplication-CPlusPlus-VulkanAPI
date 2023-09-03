#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/UI/UI.h"
#include <iostream>
#include <sqlite3.h>
#define IM_CLAMP(V, MN, MX)     ((V) < (MN) ? (MN) : (V) > (MX) ? (MX) : (V))
static bool load_open = false;
static bool menu_open = true;
static bool receipt_open = false;
static bool payment_open = false;
static bool animate = false;
static bool animate_gone = false;
static bool card = false;
static double sum = 0;
static int count = 0;
const char* text = "Connecting to DataBase...";
const char* errortext = "";
static std::string selected = "0 American";
const char* User = "";
struct FoodType {
	const char* first;
	double firstcost;
	const char* second;
	double secondcost;
	const char* third;
	double thirdcost;
	const char* fourth;
	double fourthcost;
	const char* details;
};
std::vector<std::string> orderedFoods;
std::vector<double> orderedCost;
static std::map<std::string, int> foodCounts;
int number;
std::map<std::string, FoodType> foodTypes = {
	{"0 American", {"Burger",5.25f, "Fries",2.50f, "Chicken sandwich",4.99f, "HotDog",5.20f, "5"}},
	{"1 Asian", {"California roll",15.1f, "Unagi",13.2f, "Nigirizushi",8.f, "Makizushi",10.f, "Sushi"}},
	{"2 European", {"Minestrone",5.f, "Lentil Soup",4.f, "Chicken Tortilla Soup",6.f, "French Onion Soup",7.f, "Soups"}},
	{"3 Italian", {"Margherita",12.4f, "Lasagne alla Bolognese",15.f, "Gnocchi di Patate",10.f, "Fettuccine al Pomodoro",6.f, "Pizza"}},
	{"4 Mexican", {"Tacos",8.f, "Pozole",6.f, "Sopaipillas",10.f, "Enchiladas",9.f, "Taco"}},
	{"5 Vegetarian", {"Vegetarian Meatballs",10.3f, "Falafel",5.f, "Cauliflower Curry",7.5f, "Cauliflower Pizza Crust",10.f, "Vegetables"}},
	{"6 Salads", {"Greek salad",5.f, "Herring salad",5.f, "Insalata Caprese",5.f, "Chicken salad",5.f, "Salads"}},
	{"7 Sweets", {"Chocolate Donuts 5pcs",7.5f, "Waffles & Maple syrop 3pcs",10.5f, "Chocolate Croissant 3pcs",10.f, "Apple pie 1pc",5.f, "Sweets"}},
	{"8 Ice Cream", {"Chocolate",1.75f, "Vanilla",1.25f, "Strawberry",1.5f, "Pistachio",2.3f, "Ice Creams"}},
	{"9 Drinks", {"Cola 0,5ml",3.00f, "Sprite 0,5ml",2.50f, "Black Tea",1.25f, "Coffe",2.00f, "Drinks"}},
};
static void ConnectToSQLServer() {
	sqlite3* db;
	char* errorMessage;
	std::cout << "SQLite created" << std::endl;
	int rc = sqlite3_open("CreditCard.db", &db);
	/*if (rc != SQLITE_OK) {
		std::cout << "DB can`t created: " << sqlite3_errmsg(db) << std::endl;
	}*/
	rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Payment (name TEXT, cardnumber TEXT, ExpiryDate TEXT, CVC TEXT)", 0, 0, &errorMessage);
	/*if (rc != SQLITE_OK) {
		std::cout << "Payment tables can`t created: " << errorMessage << std::endl;
		sqlite3_free(errorMessage);
	}*/
	rc = sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Clients (login TEXT, password TEXT, number TEXT)", 0, 0, &errorMessage);
	/*if (rc != SQLITE_OK) {
		std::cout << "Clients tables can`t created: " << errorMessage << std::endl;
		sqlite3_free(errorMessage);
	}*/
	if (rc == SQLITE_OK) {
		animate = true;
	}
}
class NewLayer : public Walnut::Layer
{
public:
	static void Calculate(const char* name, double cost) {
		if (foodCounts.find(name) != foodCounts.end()) {
			foodCounts[name]++;
			number = foodCounts[name]; // Set the number to the current count
		}
		else {
			foodCounts[name] = 1;
			orderedFoods.push_back(name);
			orderedCost.push_back(cost);
			number = 1; // Start the count for this new food item
		}
		sum += cost;
	}

	static void ShowLoadLayout(bool* p_open) {

		if (ImGui::Begin("Loading", p_open, ImGuiWindowFlags_NoCollapse)) {
			
			static float progress = 0.0f, progress_dir = 1.0f;
			float centerPos = (ImGui::GetWindowSize().x - 400.0f) * 0.50f;
			sqlite3* db;
			int rc = sqlite3_open("CreditCard.db", &db);
			if (rc != SQLITE_OK) {
				text = "Failed to connect to DataBase...";
			}
			if (animate_gone) {
				
				centerPos = (ImGui::GetWindowSize().x - 300.0f) * 0.50f;
				ImGui::SetCursorPosX(centerPos);
				centerPos = (ImGui::GetWindowSize().y - 150.0f) * 0.50f;
				ImGui::SetCursorPosY(centerPos);
				ImGui::Text("Welcome! Please log in or sign up.");
				centerPos = (ImGui::GetWindowSize().x - 250.0f) * 0.50f;
				ImGui::SetCursorPosX(centerPos);
				const float inputWidth = 200.0f;
				ImGui::PushItemWidth(inputWidth);
				static char username[64] = "";
				static char password[64] = "";
				static char phone_number[64] = "";
				ImGui::InputTextWithHint("##name_input", "Username..", username, 32, ImGuiInputTextFlags_EnterReturnsTrue);
				ImGui::SameLine();
				ImGui::Text(errortext);
				ImGui::SetCursorPosX(centerPos);
				ImGui::InputTextWithHint("##firstpassword", "Password..", password, 32, ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue);
				ImGui::SetCursorPosX(centerPos);
				ImGui::InputTextWithHint("##phone_number", "Phone number..", phone_number, 32, ImGuiInputTextFlags_EnterReturnsTrue);

				ImGui::SetCursorPosX(centerPos);
				if (ImGui::Button(" Sign Up"))
				{
					rc = sqlite3_exec(db, "BEGIN TRANSACTION", 0, 0, 0);
					const char* checkQuery = "SELECT login FROM Clients WHERE login = ?";
					sqlite3_stmt* checkStatement;
					rc = sqlite3_prepare_v2(db, checkQuery, -1, &checkStatement, 0);
					rc = sqlite3_bind_text(checkStatement, 1, username, -1, SQLITE_STATIC);
					if (sqlite3_step(checkStatement) == SQLITE_ROW) {
						errortext = "error, username already exists";
					}
					else {
						const char* insertQuery = "INSERT INTO Clients VALUES (?, ?, ?)";
						sqlite3_stmt* insertStatement;
						rc = sqlite3_prepare_v2(db, insertQuery, -1, &insertStatement, 0);
						rc = sqlite3_bind_text(insertStatement, 1, username, -1, SQLITE_STATIC);
						rc = sqlite3_bind_text(insertStatement, 2, password, -1, SQLITE_STATIC);
						rc = sqlite3_bind_text(insertStatement, 3, phone_number, -1, SQLITE_STATIC);
						rc = sqlite3_step(insertStatement);
						if (rc == SQLITE_DONE) {
							rc = sqlite3_finalize(insertStatement);
							rc = sqlite3_exec(db, "COMMIT", 0, 0, 0);
							errortext = "Registration successfully";
						}
						else {
							rc = sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
							errortext = "error, failed to insert data";
						}
					}
					rc = sqlite3_finalize(checkStatement);
				}
				ImGui::SameLine();
				centerPos = (ImGui::GetWindowSize().x - 10.0f ) * 0.50f;
				ImGui::SetCursorPosX(centerPos);
				if (ImGui::Button("  Log In  "))
				{
					const char* checkQuery = "SELECT login FROM Clients WHERE login = ? AND password = ? AND number = ?";
					sqlite3_stmt* checkStatement;
					rc = sqlite3_prepare_v2(db, checkQuery, -1, &checkStatement, 0);
					rc = sqlite3_bind_text(checkStatement, 1, username, -1, SQLITE_STATIC);
					rc = sqlite3_bind_text(checkStatement, 2, password, -1, SQLITE_STATIC);
					rc = sqlite3_bind_text(checkStatement, 3, phone_number, -1, SQLITE_STATIC);

					if (sqlite3_step(checkStatement) == SQLITE_ROW) {
						User = username;
						load_open = false;
						menu_open = true;
						ShowAppLayout(&menu_open);
					}
					else {
						errortext = "Invalid login credentials";
					}

					rc = sqlite3_finalize(checkStatement);
				}
			}
			else {
				if (animate) {
					progress += progress_dir * 1.8f * ImGui::GetIO().DeltaTime;
					if (progress >= +1.1f) { progress = +1.1f; animate_gone = true; }
				}
				else {
					progress += progress_dir * 0.4f * ImGui::GetIO().DeltaTime;
					if (progress >= +0.6f) { progress = +0.6f; }
					
				}
				ImGui::SetCursorPosX(centerPos);
				centerPos = (ImGui::GetWindowSize().y - 150.0f) * 0.50f;
				ImGui::SetCursorPosY(centerPos);
				ImGui::ProgressBar(progress, ImVec2(400.0f, 0.0f));
				centerPos = (ImGui::GetWindowSize().x - 200.0f) * 0.50f;
				ImGui::SetCursorPosX(centerPos);
				ImGui::Text(text);
				ConnectToSQLServer();
			}
		}
		ImGui::End();
	}

	static void ShowAppLayout(bool* p_open){
		if (ImGui::Begin("Menu", p_open, ImGuiWindowFlags_NoTitleBar))
		{
			
			{
				ImGui::BeginChild("left pane", ImVec2(300, 0), true);
				ImGui::Text("Ordering:");
				ImGui::Separator();

				for (const auto& foodType : foodTypes) {
					char label[128];
					snprintf(label, sizeof(label), "%s", foodType.first.c_str());
					if (ImGui::Selectable(label, selected == foodType.first))
						selected = foodType.first;
				}
				ImGui::EndChild();
			}
			if (foodTypes.find(selected) != foodTypes.end()) {
				ImGui::SameLine();

				ImGui::BeginGroup();
				ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));

				const FoodType& currentFood = foodTypes[selected];
				ImGui::Text("Filter: %s", selected.c_str());
				ImGui::Separator();

				if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
					if (ImGui::BeginTabItem("Description")) {
						ImGui::Separator();
						ImGui::Columns(2, "columns");
						
						ImGui::BeginGroup();
						auto image = Walnut::Application::Get().GetAmerican_Burger();
						ImGui::Image(image->GetDescriptorSet(), { 148, 148 });
						ImGui::TextWrapped("%s", currentFood.first);
						ImGui::Text("%g $", currentFood.firstcost);
						ImGui::PushID("1");
						if (ImGui::Button("Order", ImVec2(140, 40))) {
							receipt_open = true;
							if (foodTypes.find(selected) != foodTypes.end()) {
								Calculate(currentFood.first, currentFood.firstcost);
							}
						}
						ImGui::PopID();
						ImGui::EndGroup();ImGui::NextColumn();
						ImGui::BeginGroup();
						image = Walnut::Application::Get().GetAmerican_Potato();
						ImGui::Image(image->GetDescriptorSet(), { 148, 148 });
						ImGui::TextWrapped("%s", currentFood.second);
						ImGui::Text("%g $", currentFood.secondcost);
						ImGui::PushID("2");
						if (ImGui::Button("Order", ImVec2(140, 40))) {
							receipt_open = true;
							if (foodTypes.find(selected) != foodTypes.end()) {
								Calculate(currentFood.second, currentFood.secondcost);
							}
						}
						ImGui::PopID();
						ImGui::EndGroup();ImGui::NextColumn(); 
						ImGui::Separator();
						ImGui::BeginGroup();
						image = Walnut::Application::Get().GetAmerican_Sandwich();
						ImGui::Image(image->GetDescriptorSet(), { 148, 148 });
						ImGui::TextWrapped("%s", currentFood.third);
						ImGui::Text("%g $", currentFood.thirdcost);
						ImGui::PushID("3");
						if (ImGui::Button("Order", ImVec2(140, 40))) {
							receipt_open = true;
							if (foodTypes.find(selected) != foodTypes.end()) {
								Calculate(currentFood.third, currentFood.thirdcost);
							}
						}
						ImGui::PopID();
						ImGui::EndGroup();ImGui::NextColumn(); 
						ImGui::BeginGroup();
						image = Walnut::Application::Get().GetAmerican_HotDog();
						ImGui::Image(image->GetDescriptorSet(), { 148, 148 });
						ImGui::TextWrapped("%s", currentFood.fourth);
						ImGui::Text("%g $", currentFood.fourthcost);
						ImGui::PushID("4");
						if (ImGui::Button("Order", ImVec2(140, 40))) {
							receipt_open = true;
							if (foodTypes.find(selected) != foodTypes.end()) {
								Calculate(currentFood.fourth, currentFood.fourthcost);
							}
						}
						ImGui::PopID();
						ImGui::EndGroup();ImGui::NextColumn(); 
						ImGui::EndTabItem();ImGui::Separator();
					}
					if (ImGui::BeginTabItem("Details")) {
						ImGui::Text("%s", currentFood.details);
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
				ImGui::EndChild();
				if (ImGui::Button("Revert")) {}
				ImGui::SameLine();
				if (ImGui::Button("Save")) {}
				ImGui::EndGroup();
			}
		}
		ImGui::End();
	}
	static bool IsValidCardNumber(const char* cardnumber) {
		if (cardnumber[0] == '2' || cardnumber[0] == '4' || cardnumber[0] == '5') {
			int length = 0;
			for (int i = 0; cardnumber[i]; i++) {
				if (cardnumber[i] != ' ') {
					length++;
					if (!isdigit(cardnumber[i])) {
						return false; 
					}
				}
			}
			return length == 16; 
		}
		return false; 
	}
	static bool IsValidExpiryDate(const char* expiryDate) {
		int month, year;
		if (sscanf(expiryDate, "%d/%d", &month, &year) != 2) {
			return false; // Invalid format
		}
		// Check if the month and year are within valid ranges
		return (month >= 1 && month <= 12) && (year >= 0 && year <= 99);
	}
	static void ShowPayment(bool* p_open) {
		if (ImGui::Begin("Payment", p_open, ImGuiWindowFlags_NoTitleBar))
		{
			float old_size = ImGui::GetFont()->Scale;
			ImGui::GetFont()->Scale += 0.2f;
			ImGui::PushFont(ImGui::GetFont());
			float centerPos = (ImGui::GetWindowSize().x - 100.f) * 0.50f;
			ImGui::SetCursorPosX(centerPos);
			//0.3921568627450980392156862745098 color
			ImGui::TextColored(ImVec4(0.341f, 0.51f, 0.17f, 1.0f), "Total: %g$", sum);
			ImGui::TextColored(ImVec4(0.341f, 0.51f, 0.17f, 1.0f), "Choose card to pay.");
			ImGui::GetFont()->Scale = old_size;
			ImGui::PopFont();
			sqlite3* db;
			int rc = sqlite3_open("CreditCard.db", &db);
			if (rc != SQLITE_OK) {
				text = "Failed to connect to DataBase...";
			}
			const char* selectQuery = "SELECT cardnumber, ExpiryDate FROM Payment WHERE name = ?";
			sqlite3_stmt* selectStatement;
			rc = sqlite3_prepare_v2(db, selectQuery, -1, &selectStatement, 0);
			rc = sqlite3_bind_text(selectStatement, 1, User, -1, SQLITE_STATIC);

			std::vector<std::pair<std::string, std::string>> existingCards;

			while (sqlite3_step(selectStatement) == SQLITE_ROW) {
				const char* cardNumber = (const char*)sqlite3_column_text(selectStatement, 0);
				const char* expiryDate = (const char*)sqlite3_column_text(selectStatement, 1);

				if (cardNumber && expiryDate) {
					existingCards.emplace_back(std::make_pair(std::string(cardNumber), std::string(expiryDate)));
				}
			}
			sqlite3_finalize(selectStatement);
				if (!existingCards.empty()) {
					int countcards = 0;
					for (const auto& cardInfo : existingCards) {
						countcards++;
						std::string Childname = cardInfo.first;
						ImGui::BeginGroup();
						ImGui::BeginChild(Childname.c_str(), ImVec2(350.0f, 200.0f));
						if (cardInfo.first[0] == '2' || cardInfo.first[0] == '5') {
							centerPos = (ImGui::GetWindowSize().y + 50.f) * 0.50f;
							ImGui::SetCursorPosY(centerPos);
							centerPos = (ImGui::GetWindowSize().x + 180.f) * 0.50f;
							ImGui::SetCursorPosX(centerPos);
							auto image = Walnut::Application::Get().GetMastercard();
							ImGui::Image(image->GetDescriptorSet(), { 74, 74 });
						}
						else if (cardInfo.first[0] == '4') {
							centerPos = (ImGui::GetWindowSize().y + 50.f) * 0.50f;
							ImGui::SetCursorPosY(centerPos);
							centerPos = (ImGui::GetWindowSize().x + 180.f) * 0.50f;
							ImGui::SetCursorPosX(centerPos);
							auto image = Walnut::Application::Get().GetVisa_card();
							ImGui::Image(image->GetDescriptorSet(), { 74, 74 });
						}
						else if (cardInfo.first[0] == '3') {
							centerPos = (ImGui::GetWindowSize().y + 50.f) * 0.50f;
							ImGui::SetCursorPosY(centerPos);
							centerPos = (ImGui::GetWindowSize().x + 180.f) * 0.50f;
							ImGui::SetCursorPosX(centerPos);
							auto image = Walnut::Application::Get().GetAmericanExpress();
							ImGui::Image(image->GetDescriptorSet(), { 74, 74 });
						}
						ImGui::PushItemWidth(235.0f);
						centerPos = (ImGui::GetWindowSize().y - 0.f) * 0.50f;
						ImGui::SetCursorPosY(centerPos);
						centerPos = (ImGui::GetWindowSize().x - 270.f) * 0.50f;
						ImGui::SetCursorPosX(centerPos);
						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
						ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.6f));
						old_size = ImGui::GetFont()->Scale;
						ImGui::GetFont()->Scale += 0.25f;
						ImGui::PushFont(ImGui::GetFont());

						ImGui::Text(cardInfo.first.c_str());

						ImGui::GetFont()->Scale = old_size;
						ImGui::PopFont();
						ImGui::PopStyleColor();
						ImGui::PopStyleVar();
						centerPos = (ImGui::GetWindowSize().y + 100.f) * 0.50f;
						ImGui::SetCursorPosY(centerPos);
						centerPos = (ImGui::GetWindowSize().x - 150.f) * 0.50f;
						ImGui::SetCursorPosX(centerPos);
						ImGui::PushItemWidth(70.0f);
						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
						ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.6f));

						ImGui::Text(cardInfo.second.c_str());

						ImGui::PopStyleColor();
						ImGui::PopStyleVar();
						ImGui::EndChild(); 
						ImGui::EndGroup();
						ImGui::SameLine();
						ImGui::BeginChild(Childname.c_str(), ImVec2(350.0f, 200.0f));

						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
						centerPos = (ImGui::GetWindowSize().y - 200.f);
						ImGui::SetCursorPosY(centerPos);
						if (ImGui::Button(Childname.c_str(), ImVec2(350.0f, 200.0f))) {

						}
						ImGui::PopStyleColor(3);
						ImGui::PopStyleVar();

						ImGui::EndChild();
						if (countcards % 2 == 0) {
							ImGui::Spacing();
						}
						else {
							ImGui::SameLine();
						}
						
						
						
						}

					existingCards.clear();
				}
					

			static char cardnumber[20] = "";
				static char ExpiryDate[6] = "";
				static char CVC[4] = "";
			
			bool realnumber = false;
			bool realdate = false;
			if (card) {
				ImGui::Spacing();
				ImGui::BeginGroup();
				ImGui::BeginChild("new card", ImVec2(350.0f, 200.0f));
				if (cardnumber[0] == '2' || cardnumber[0] == '5') {
					centerPos = (ImGui::GetWindowSize().y + 50.f) * 0.50f;
					ImGui::SetCursorPosY(centerPos);
					centerPos = (ImGui::GetWindowSize().x + 180.f) * 0.50f;
					ImGui::SetCursorPosX(centerPos);
					auto image = Walnut::Application::Get().GetMastercard();
					ImGui::Image(image->GetDescriptorSet(), { 74, 74 });
					errortext = "";
					realnumber = true;
				}
				else if (cardnumber[0] == '4') {
					centerPos = (ImGui::GetWindowSize().y + 50.f) * 0.50f;
					ImGui::SetCursorPosY(centerPos);
					centerPos = (ImGui::GetWindowSize().x + 180.f) * 0.50f;
					ImGui::SetCursorPosX(centerPos);
					auto image = Walnut::Application::Get().GetVisa_card();
					ImGui::Image(image->GetDescriptorSet(), { 74, 74 });
					errortext = "";
					realnumber = true;
				}
				else if (cardnumber[0] == '3') {
					centerPos = (ImGui::GetWindowSize().y + 50.f) * 0.50f;
					ImGui::SetCursorPosY(centerPos);
					centerPos = (ImGui::GetWindowSize().x + 180.f) * 0.50f;
					ImGui::SetCursorPosX(centerPos);
					auto image = Walnut::Application::Get().GetAmericanExpress();
					ImGui::Image(image->GetDescriptorSet(), { 74, 74 });
					errortext = "";
					realnumber = true;
				}
				else {
					if (strlen(cardnumber) == 1) {
						errortext = "Invalid card number, your number need to start from:\n"
						"2 or 5 - Mastercard, 4 - Visa, 3 - American Express";
						realnumber = false;
					}
				}
				ImGui::PushItemWidth(235.0f);
				centerPos = (ImGui::GetWindowSize().y - 0.f) * 0.50f;
				ImGui::SetCursorPosY(centerPos);
				centerPos = (ImGui::GetWindowSize().x - 270.f) * 0.50f;
				ImGui::SetCursorPosX(centerPos);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f); 
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.6f));
				old_size = ImGui::GetFont()->Scale;
				ImGui::GetFont()->Scale += 0.25f;
				ImGui::PushFont(ImGui::GetFont());
				ImGui::InputTextWithHint("##number_input", "1234 5678 9012 3456", cardnumber, 20, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AlwaysOverwrite);
				ImGui::GetFont()->Scale = old_size;
				ImGui::PopFont();
				ImGui::PopStyleColor(); 
				ImGui::PopStyleVar();
				ImGui::SameLine();
				centerPos = (ImGui::GetWindowSize().y + 100.f) * 0.50f;
				ImGui::SetCursorPosY(centerPos);
				centerPos = (ImGui::GetWindowSize().x - 150.f) * 0.50f;
				ImGui::SetCursorPosX(centerPos);
				ImGui::PushItemWidth(70.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.6f));
				ImGui::InputTextWithHint("##data", "09/25", ExpiryDate, 6, ImGuiInputTextFlags_EnterReturnsTrue);
				if (!IsValidCardNumber(cardnumber)) {
					errortext = "Invalid card number. Please enter a valid card number.";
					realnumber = false;
				}
				else if (!IsValidExpiryDate(ExpiryDate)) {
					errortext = "Invalid expiry date. Please enter a valid expiry date (MM/YY).";
					realdate = false;
				}
				else {
					errortext = "";
					realnumber = true;
					realdate = true;
				}
				ImGui::PopStyleColor();
				ImGui::PopStyleVar();
				ImGui::EndChild();
				
				ImGui::SameLine();
				
				ImGui::BeginChild("item views", ImVec2(350.0f, 200.0f));
				centerPos = (ImGui::GetWindowSize().y - 165.f) * 0.50f;
				ImGui::SetCursorPosY(centerPos);
				centerPos = (ImGui::GetWindowSize().x - 1000.f) * 0.50f;
				ImGui::SetCursorPosX(centerPos);

				ImDrawList* draw_list = ImGui::GetWindowDrawList();
				static ImVec4 colf = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
				const ImU32 col = ImColor(colf);
				const ImVec2 p = ImGui::GetCursorScreenPos();
				float x = p.x + 4.0f;
				float y = p.y + 4.0f;
				draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + 1000.f, y + 30.f), col);
				centerPos = (ImGui::GetWindowSize().y - 25.f) * 0.50f;
				ImGui::SetCursorPosY(centerPos);
				centerPos = (ImGui::GetWindowSize().x + 100.f) * 0.50f;
				ImGui::SetCursorPosX(centerPos);
				ImGui::PushItemWidth(60.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.6f));
				ImGui::InputTextWithHint("##cvc", " CVC ", CVC, 4, ImGuiInputTextFlags_EnterReturnsTrue);
				ImGui::PopStyleColor();
				ImGui::PopStyleVar();
				ImGui::EndChild();
				ImGui::EndGroup();
			}
			ImGui::Spacing();
			if (ImGui::Button("Add card")) {
				card = true;
				if (card && strlen(cardnumber) == 19 && strlen(ExpiryDate) == 5 && strlen(CVC) == 3 && realnumber && realdate) {
					const char* insertQuery = "INSERT INTO Payment VALUES (?, ?, ?, ?)";
					sqlite3_stmt* insertStatement;
					const char* name = User;
					rc = sqlite3_prepare_v2(db, insertQuery, -1, &insertStatement, 0);
					rc = sqlite3_bind_text(insertStatement, 1, name, -1, SQLITE_STATIC);
					rc = sqlite3_bind_text(insertStatement, 2, cardnumber, -1, SQLITE_STATIC);
					rc = sqlite3_bind_text(insertStatement, 3, ExpiryDate, -1, SQLITE_STATIC);
					rc = sqlite3_bind_text(insertStatement, 4, CVC, -1, SQLITE_STATIC);
					rc = sqlite3_step(insertStatement);
					if (rc == SQLITE_DONE) {
						rc = sqlite3_finalize(insertStatement);
						rc = sqlite3_exec(db, "COMMIT", 0, 0, 0);
						errortext = "Card added";
						card = false;
					}
					else {
						rc = sqlite3_exec(db, "ROLLBACK", 0, 0, 0);
						errortext = "Error";
					}
				}
			}
			ImGui::Text(errortext);
		}
		ImGui::End();
	}
	static void ShowReceipt(bool* p_open) {
		if (!ImGui::Begin("E-receipt", p_open, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::End();
			return;
		}
		float old_size = ImGui::GetFont()->Scale;
		ImGui::GetFont()->Scale += 0.2f;
		ImGui::PushFont(ImGui::GetFont());
		float centerPos = (ImGui::GetWindowSize().x - 150.f) * 0.50f;
		ImGui::SetCursorPosX(centerPos);
		ImGui::TextColored(ImVec4(0.64f, 0.04f, 0.15f, 1.0f), "24 Food Delivery");
		ImGui::GetFont()->Scale = old_size;
		ImGui::PopFont();

		ImGui::GetFont()->Scale += 0.2f;
		ImGui::PushFont(ImGui::GetFont());
		centerPos = (ImGui::GetWindowSize().x - 350.f) * 0.50f;
		ImGui::SetCursorPosX(centerPos);
		ImGui::Text("Address: 201 Mission St, San Francisco\n");
		ImGui::GetFont()->Scale = old_size;
		ImGui::PopFont();

		ImGui::TextUnformatted(
			"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n"
		);
		ImGui::GetFont()->Scale += 0.2f;
		ImGui::PushFont(ImGui::GetFont());
		centerPos = (ImGui::GetWindowSize().x - 100.f) * 0.50f;
		ImGui::SetCursorPosX(centerPos);
		ImGui::Text("E-RECEIPT");
		ImGui::GetFont()->Scale = old_size;
		ImGui::PopFont();
		ImGui::TextUnformatted(
			"* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n"
		);
		std::vector<std::string> itemsToDelete;
		int itemCount;
		std::string itemName;
		for (size_t i = 0; i < orderedFoods.size(); i++) {
			if (orderedFoods[i] != "") {
				const std::string& orderedFood = orderedFoods[i];
				const float orderedcost = orderedCost[i];

				auto it = foodCounts.find(orderedFood);



				if (it != foodCounts.end()) {
					itemCount = it->second;
					itemName = it->first;

					auto typeIt = foodTypes.find(selected);
					if (typeIt != foodTypes.end()) {
						//const FoodType& foodType = typeIt->second;
						if (orderedFood != "") {
							ImGui::Columns(2, "columns");

							ImGui::Text("%s", orderedFood.c_str()); ImGui::NextColumn();
							ImGui::Text("%d", itemCount);
							ImGui::SameLine();
							ImGui::Text("x%g", orderedcost);
							ImGui::SameLine();
							double multiplication = itemCount * orderedcost;
							ImGui::Text("= %g$", multiplication);
							ImGui::SameLine();
							ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.f, 0.f, 0.f, 0.0f));
							ImGui::BeginChild(orderedFood.c_str(), ImVec2(20.f, 20.f));
							ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.2f));
							ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
							centerPos = (ImGui::GetWindowSize().x - 20.f);
							ImGui::SetCursorPosX(centerPos);
							auto image = Walnut::Application::Get().GetIconClose();
							ImGui::Image(image->GetDescriptorSet(), { 20, 20 });
							centerPos = (ImGui::GetWindowSize().y - 20.f);
							ImGui::SetCursorPosY(centerPos);
							centerPos = (ImGui::GetWindowSize().x - 20.f);
							ImGui::SetCursorPosX(centerPos);
							if (ImGui::Button(orderedFood.c_str(), ImVec2(20.f, 20.f))) {
								/*itemsToDelete.push_back(itemName);
								sum -= (orderedcost)*itemCount;*/
							}
							ImGui::PopStyleColor(3);
							ImGui::PopStyleVar();
							ImGui::EndChild();
							ImGui::PopStyleColor();
							ImGui::NextColumn();
							ImGui::Separator();
						}
					}
				}
			}
		}
		/*for (const std::string& item : itemsToDelete) {
			auto foundItem = std::find(orderedFoods.begin(), orderedFoods.end(), item);
			if (foundItem != orderedFoods.end()) {
				int index = std::distance(orderedFoods.begin(), foundItem);
				sum -= orderedCost[index] * foodCounts[item];
				orderedFoods.erase(foundItem);
				foodCounts[item] = 0;
				orderedCost.erase(orderedCost.begin() + index);
			}
		}

		itemsToDelete.clear();*/

		ImGui::Columns(1);
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::GetFont()->Scale +=0.2f;
		ImGui::PushFont(ImGui::GetFont());
		centerPos = (ImGui::GetWindowSize().x - 100.f) * 0.50f;
		ImGui::SetCursorPosX(centerPos);
		//0.3921568627450980392156862745098 color
		ImGui::TextColored(ImVec4(0.341f, 0.51f, 0.17f, 1.0f), "Total: %g$", sum);
		ImGui::GetFont()->Scale = old_size;
		ImGui::PopFont();
		if (ImGui::Button("Order", ImVec2(100.f, 40.f))) {
			payment_open = true;
			ImGui::SetWindowFocus("Payment");
		}
			
			
		ImGui::End();
	}
	virtual void OnUIRender() override
	{
		//ImGui::ShowDemoWindow();
		if (load_open)
			ShowLoadLayout(&load_open);
		if (menu_open)              
			ShowAppLayout(&menu_open);
		if (receipt_open) {
			ImGui::SetNextWindowSizeConstraints(ImVec2(500.0f, 100.0f), ImVec2(800.0f,800.0f));
			ShowReceipt(&receipt_open);
		}
		if (payment_open)
			ShowPayment(&payment_open);
			
		UI_DrawAboutModal();
	}

	void UI_DrawAboutModal()
	{
		if (!m_AboutModalOpen)
			return;

		ImGui::OpenPopup("About");
		m_AboutModalOpen = ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
		if (m_AboutModalOpen)
		{

			auto image = Walnut::Application::Get().GetApplicationIcon();
			ImGui::Image(image->GetDescriptorSet(), { 48, 48 });

			ImGui::SameLine();
			Walnut::UI::ShiftCursorX(20.0f);

			ImGui::BeginGroup();
			ImGui::Text("Application created");
			ImGui::Text("by Alex Chazov.");
			ImGui::EndGroup();

			if (Walnut::UI::ButtonCentered("Close"))
			{
				m_AboutModalOpen = false;
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	void ShowAboutModal()
	{
		m_AboutModalOpen = true;
	}
private:
	bool m_AboutModalOpen = false;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "24 Food Delivery";
	spec.CustomTitlebar = true;

	Walnut::Application* app = new Walnut::Application(spec);
	std::shared_ptr<NewLayer> Layer = std::make_shared<NewLayer>();
	app->PushLayer(Layer);
	app->SetMenubarCallback([app, Layer]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				Layer->ShowAboutModal();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}