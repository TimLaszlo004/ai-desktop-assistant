from win11toast import toast

def on_destructive_toast(function:str):
    accepted = False
    def accept(args):
        nonlocal accepted
        accepted = True

    toast('Assistant', f'Click here to allow execution of destructive function:\n{function}', on_click=accept)
    return accepted

# print(on_destructive_toast("google"))