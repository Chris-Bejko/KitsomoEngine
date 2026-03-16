#pragma once
#include "Command.h"
#include <stack>
#include <memory>

class CommandHistory
{
public:
	static CommandHistory& get()
	{
		static CommandHistory instance;
		return instance;
	}

	void Execute(std::unique_ptr<Command> command)
	{
		command->Execute();
		undoStack.push(std::move(command));
		
		// Clear redo stack when new command is executed
		while (!redoStack.empty())
			redoStack.pop();
	}

	void Undo()
	{
		if (undoStack.empty()) return;
		
		auto command = std::move(undoStack.top());
		undoStack.pop();
		command->Undo();
		redoStack.push(std::move(command));
	}

	void Redo()
	{
		if (redoStack.empty()) return;
		
		auto command = std::move(redoStack.top());
		redoStack.pop();
		command->Execute();
		undoStack.push(std::move(command));
	}

	bool CanUndo() const { return !undoStack.empty(); }
	bool CanRedo() const { return !redoStack.empty(); }

	std::string GetUndoDescription() const
	{
		if (undoStack.empty()) return "";
		return undoStack.top()->GetDescription();
	}

	std::string GetRedoDescription() const
	{
		if (redoStack.empty()) return "";
		return redoStack.top()->GetDescription();
	}

	void Clear()
	{
		while (!undoStack.empty()) undoStack.pop();
		while (!redoStack.empty()) redoStack.pop();
	}

private:
	CommandHistory() = default;
	std::stack<std::unique_ptr<Command>> undoStack;
	std::stack<std::unique_ptr<Command>> redoStack;
};
