function updatelabel() {
    var label = document.getElementById('edit-label');
    var title = document.getElementById('edit-titleinfo-title');
    var subti = document.getElementById('edit-titleinfo-subtitle');
    label.value = title.value + ' ' + subti.value;
}
